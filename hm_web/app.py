#!/usr/bin/env python
# -*- coding: utf-8 -*-
from __future__ import division
try:
    import couchbase
    from couchbase.bucket import Bucket
    from couchbase.views.iterator import View
except:
    import couchbase_ffi
    from couchbase_ffi.bucket import Bucket
    from couchbase_ffi.views.iterator import View
import random
import string
import web
import json
import xml.etree.ElementTree
from all_tags import tagsDict
import re
import os
import sys
from generate import gen
from subprocess import Popen, PIPE

ERR_0 = 0
ERR_1 = 1
ERR_2 = 2
ERR_3 = 3

web.config.debug = False

urls = ('/', 'home',
        '/signup', 'signup',
        '/signin', 'signin',
        '/mod/create', 'mod_create',
        '/mod/edit/.*', 'mod_edit',
        '/mod', 'mod',
        '/deck/.*', 'deck_edit',
        '/faq', 'faq',
        '/rss', 'rss',
        '/ladder', 'ladder'
        )
render = web.template.render('templates/')

app = web.application(urls, globals())

store = web.session.DiskStore('sessions')
session = web.session.Session(app, store, initializer={'login': 0})

cb = Bucket('couchbase://localhost/hbs', password='aci')

def rand(num):
    return ''.join([random.choice(string.ascii_letters + string.digits) for n in xrange(num)])

class faq:
    def GET(self):
        return render.base(session, render.faq(), [], [])

class mod_edit:
    def GET(self):
        s =  web.ctx['path'].split("/")
        m = s[len(s) - 1]
        print m
        return render.base(session, [], [], [])

class ladder:
    def GET(self):
        players = []
        for result in View(cb, "dev_result", "result"):
            w = int(result.value["w"])
            l = int(result.value["l"])
            n = result.value["nick"]
            players.append([n, w, l, int(round(w / (w + l), 2) * 100)])
        players = sorted(players, key=lambda x: x[3], reverse=True)

        return render.base(session, render.ladder(players), [], [])

class home:
    def GET(self):
        return render.base(session, render.home(), [], [])

class mod:
    def __init__(self):
        print 'mod init'

    def POST(self):
        #try:
            d = json.loads(web.data())
            if(len(d['name']) > 0 and len(d['url']) > 0):
                mod = {'name': d['name'], 'url': d['url'], 'cards': []}
                cb.upsert('u:mod_%s' % d['name'], mod)

                return 'success'

            return 'fail'

        #except:
        #    print 'except'
        #    return ''

    def GET(self):
        if(session.login == 0):
            return render.base(session, render.login_required(), [], [])

        l = []
        for result in View(cb, "dev_mods", "mods"):
            l.append({ "url": result.value["url"], "name": result.value["name"] })

        return render.base(session, render.mod(l), [], [])

class mod_edit:
    def __init__(self):
        self.e = xml.etree.ElementTree.parse('cache/c0').getroot()
        self.cards = self.defaultSchema()
        s =  web.ctx['path'].split("/")
        self.modname = "u:mod_%s" % s[len(s) - 1]

    def defaultSchema(self):
        f = open('cache/cards.basic')
        default = f.read()
        f.close()

        alias = []
        for atype in self.e.findall("Entity"):
            card = atype.get("CardID")
            if(default.find("[%s]" % card) == -1):
                continue
            for t in atype.findall("Tag"):
                if(t.get("enumID") == "185"):
                    alias.append([card, t.text])
        return alias

    def cardTags(self, dst, e):
        tags = []
        tags.append(["__name", dst, "cardcode"])
        for atype in e.findall("Entity"):
            if(atype.get("CardID") == dst):
                for t in atype.findall("Tag"):
                    key = t.get("enumID")
                    m = t.get("type")
                    if(m == ""):
                        tags.append([key, t.get("value"), tagsDict[int(key)]])
                    elif(m == "String"):
                        tags.append([key, t.text, tagsDict[int(key)]])
                    else:
                        abort()
        try:
            f = open("../hm_gameserver/script/mechanics/cards/%s.card" % dst, "r")
            r = f.read()
            f.close()
            tags.append(['__code', r, 'mechanics'])
        except:
            tags.append(['__code', "{}", 'mechanics'])

        j = json.dumps(tags)

        return j

    def modCards(self, card, tags, enabledCards):

        customName = ''
        b = 'card=%s tags={' % card
        for t in tags:
            if(t.get("type") == ""):
                b += "%s:%s," % (t.get("enumID"), t.get("value"))
            if(t.get("type") == "String" and t.get("enumID") == "185"):
                customName = t.text
            if(t.get("type") == "String" and t.get("enumID") == "184"):
                r = re.findall("\$[\w]+\ ", t.text)
                if(len(r) > 0):
                    b += "47:%s," % r[0].replace(" ", "").replace("$", "")
       
        b += '} custom=1'
        enabledCards = enabledCards.replace("[%s]" % card, "name=\"%s\" %s" % (customName, b))
        return enabledCards

    def modPublish(self, modName):
        try:
            print 'publishing mod ' + self.modname
            loaded = cb.get(self.modname).value
            names = []

            f = open('cache/cards.basic')
            enabledCards = f.read()
            f.close()

            for atype in self.e.findall("Entity"):
                for new in loaded['cards']:
                    if(atype.get("CardID") == new[0][1]):
                        for t in atype.findall("Tag"):
                            for tag in new:
                                if(tag[0] == t.get("enumID")):

                                    key = t.get("enumID")
                                    m = t.get("type")

                                    if(m == ""):
                                        t.set("value", tag[1])
                                    elif(m == "String"):
                                        t.text = tag[1]
                                    else:
                                        abort()

                        enabledCards = self.modCards(new[0][1], atype.findall("Tag"), enabledCards)
                        break
            f = open(self.modname[2:], "w")
            f.write(enabledCards)
            f.close()

            xmlstr = xml.etree.ElementTree.tostring(self.e, encoding='utf8', method='xml')

            f = open("cache/cardsxml_p1", "rb")
            p1 = f.read()
            f.close()

            f = open("cache/cardsxml_p3", "rb")
            p3 = f.read()
            f.close()

            f = open("xml_%s" % self.modname[2:], "w")
            f.write(p1 + xmlstr[38:] + p3)
            f.close()


            return "success"

        except:
            return "fail"

    def enabledCardRemove(self, j):
        try:
            loaded = cb.get(self.modname).value
            # delete if element exists
            for l in loaded['cards']:
                if l[0][1] == j:
                    loaded['cards'].remove(l)
                    break
            cb.upsert(self.modname, loaded)

            names = []
            for l in loaded['cards']:
                names.append([l[0][1], l[1][1]])

            return json.dumps(names)

        except:
            cb.upsert(self.modname, {"cards": [j]})

        return ""

    def getStored(self):
        try:
            x = cb.get(self.modname)
            loaded = x.value
        except:
            return []

        names = []
        for l in loaded['cards']:
            names.append([l[0][1], l[1][1]])

        return json.dumps(names)

    def storeLoaded(self, j):
        try:
            loaded = cb.get(self.modname).value
            # replace if element exists
            for l in loaded['cards']:
                if l[0][1] == j[0][1]:
                    loaded['cards'].remove(l)
                    break
            loaded['cards'].append(j)
            cb.upsert(selfmodname, loaded)
        except:
            cb.upsert(selfmodname, {"cards": [j]})

        loaded = cb.get(self.modname).value

        names = []
        for l in loaded['cards']:
            names.append([l[0][1], l[1][1]])

        gen(j);

        return json.dumps(names)

    def modifiedCardTags(self, j):
        #try:
            loaded = cb.get(self.modname).value
            for l in loaded['cards']:
                if l[0][1] == j:
                    names = []
                    for i in l:
                        if(i[0] == '__name'):
                            names.append([i[0], i[1], "cardcode"])
                        elif(i[0] == '__code'):
                            names.append([i[0], i[1], "mechanics"])
                        else:
                            names.append([i[0], i[1], tagsDict[int(i[0])]])

                    return json.dumps(names)
                    
            return ""

    def isAuth(self):
        if session.login == 1 and session.user == 'john':
            print 'auth yes'
            return 1
        else:
            print 'auth no'
            return 0


    def POST(self):

        j = json.loads(web.data())

        if(j == None):
            return '{}'

        if(len(j) == 1):
            if 'modified' in j:
                return self.modifiedCardTags(j['modified'])

            elif 'card' in j:
                return self.cardTags(j['card'], self.e)

            elif 'remove' in j:
                if not self.isAuth():
                    return '{}'
                return self.enabledCardRemove(j['remove'])

            elif 'publish' in j:
                if not self.isAuth():
                    return '{}'

                result = self.modPublish(j['publish'])
                if(result == "success"):
                    os.system("cd script && bash publish.sh")

                return result

        else:
            if(self.isAuth()):
                return self.storeLoaded(j)
            else:
                return '{}'

    def GET(self):
        if(session.login == 0):
            return render.base(session, render.login_required(), [], [])

        try:
            stored = json.loads(self.getStored())
        except:
            stored = '{}'
        return render.base(session, render.mod_create(self.cards, len(self.cards), stored), [], [])


class signup:

    def signup(self, arr):
        session.login = 0
    
        e = arr['email']
        p = arr['pass']
        p1 = arr['pass1']

        if(len(e) < 2 or len(p) < 2):
            return ERR_1
        if(p != p1):
            return ERR_1
        
        cb.upsert('u:%s' % e, {'email': e, 'password': p, 'secret': rand(6)})
        
        return ERR_0

    def GET(self):
        return render.base(session, render.signup(), [], [])

    def POST(self):

        r = self.signup(json.loads(web.data()))
        return r

class signin:
    def signin(self, arr):
        session.login = 0
        session.user = ''
        e = arr['user']
        p = arr['pass']
        if(len(e) < 2 or len(p) < 2):
            session.login = 0
            return ERR_1

        try:
            out = cb.get('u:%s' % e).value
            for o in out:
                if(o == 'password'):
                    if(p == out[o]):
                        session.login = 1
                        session.secret = out['secret']
                        session.user = e
                        return ERR_0
                    else:
                        session.login = 0
                        return ERR_3
            return ERR_2
        except:
            return ERR_2

    def GET(self):
        return render.base(session, render.signin(), [], [])

    def POST(self):
        d = json.loads(web.data())

        r = self.signin(d)
        return r

class deck:
    def __init__(self):
        s =  web.ctx['path'].split("/")
        self.modname = s[len(s) - 1]

    def save_deck(self, arr):

        out = cb.set('u:deck_%s_%s' % (self.modname, session.secret), arr)

        return ERR_0


    def GET(self):
        if(session.login == 0):
            return render.base(session, render.login_required(), [], [])

        cards = []
        loaded = cb.get('u:mod_%s' % self.modname).value
        for l in loaded['cards']:
            names = []
            enabled = 0
            for i in l:
                if(i[0] == '185'):
                    names.append(i[1])
                elif(i[0] == '__name'):
                    names.append(i[1])
                elif(i[0] == '__code'):
                    if(i[1].find("\"enabled") > -1):
                        enabled = 1
                        if(i[1].find("\"enabled:2\"") > -1):
                            names.append(2)
                        else:
                            names.append(1)
                    if(i[1].find("\"boss\"") > -1):
                        names.append('boss')
                    else:
                        names.append('noboss')

            if(len(names) != 0 and enabled == 1):
                cards.append(names)

        try:
            loaded = cb.get('u:deck_%s_%s' % (self.modname, session.secret)).value
        except:
            loaded = {}

        return render.base(session, render.deck(cards, loaded), loaded, cards)

    def POST(self):
        if(session.login == 0):
            return render.base(session, render.login_required(), [], [])

        r = self.save_deck(json.loads(web.data()))
        return r
    
if __name__ == '__main__':
    web.wsgi.runwsgi = lambda func, addr=None: web.wsgi.runfcgi(func, addr)
    os.system("cd ../hm_sunwell/examples && npm start &")

    app.run()
