import urllib2
import os
from time import sleep

def gen(inp):
    type = ""
    name = ""
    inhand = ""
    health = ""
    attack = ""
    cost = ""
    rarity = ""
    cardname = ""
    durability = ""
    hero = ""

    for l in inp:
        if(l[0] == '202'):
            type = l[1]
        elif(l[0] == '__name'):
            name = l[1]
        elif(l[0] == '184'):
            inhand = l[1].replace("\n", " ")
        elif(l[0] == '48'):
            cost = l[1]
        elif(l[0] == '47'):
            attack = l[1]
        elif(l[0] == '45'):
            health = l[1]
        elif(l[0] == '187'):
            durability = l[1]
        elif(l[0] == '203'):
            rarity = l[1]
        elif(l[0] == '185'):
            cardname = l[1]
        elif(l[0] == '__code'):
            if(l[1].find("\"boss\"") > -1):
                hero = 'HERO'


    out = "{"

    if(len(cost) > 0):
        out +=  "\"cost\":%s," % cost

    if(type == '5'):
        out += "\"type\":\"SPELL\","
    elif(type == '4'):
        out += "\"type\":\"MINION\","
        out +=  "\"attack\":%s," % attack
        out +=  "\"health\":%s," % health
        if(len(hero) > 0):
            out +=  "\"race\":\"%s\"," % hero
    elif(type == '7'):
        out += "\"type\":\"WEAPON\","
        out +=  "\"attack\":%s," % attack
        out +=  "\"durability\":%s," % durability
    elif(type == '6'):
        return

    elif(type == '10'):
        out += "\"type\":\"HEROPOWER\","

    elif(type == '3'):
        out += "\"type\":\"MINION\","
        out +=  "\"attack\":0,"
        out +=  "\"health\":%s," % health
        out +=  "\"rarity\":5,"
        out +=  "\"cost\":0,"
    else:
        print 'wrong ', type
        exit(1)

    out += "\"texture\":\"%s\"," % name

    out += "\"id\":\"%s\"," % name

    out += "\"text\":\"%s\"," % inhand

    out += "\"name\":\"%s\"," % cardname 

    r = ""
    if(rarity == '1'):
        r = "FREE"
    elif(rarity == '2'):
        r = "COMMON"
    elif(rarity == '3'):
        r = "COMMON"
    elif(rarity == '4'):
        r = "RARE"
    elif(rarity == '5'):
        r = "LEGENDARY"

    out += "\"rarity\":\"%s\"," % r

    out +="\
      \"artist\":\"Zoltan Boros\",\
      \"set\":\"CORE\",\
      \"flavor\":\"asdf\",\
      \"playRequirements\":{\"REQ_TARGET_TO_PLAY\":0,\"REQ_MINION_TARGET\":0},\
      \"collectible\":true,\
      \"playerClass\":\"NEUTRAL\",\
      \"howToEarnGolden\":\"Unlocked at Level 45.\",\
      \"howToEarn\":\"Unlocked at Level 1.\"\
  }"

    f = open("../hm_sunwell/examples/carddata", "w")
    f.write(out)
    f.close()

    urllib2.urlopen("http://localhost:8081/").read()
