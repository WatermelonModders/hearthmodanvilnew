try:
    import couchbase
    from couchbase.bucket import Bucket
    from couchbase.views.iterator import View
except:
    import couchbase_ffi
    from couchbase_ffi.bucket import Bucket
    from couchbase_ffi.views.iterator import View

cb = Bucket('couchbase://localhost/hbs', password='aci')

def prepare_mods():
    l = []
    for result in View(cb, "dev_mods", "mods"):
        l.append({ "url": result.value["url"], "name": result.value["name"] })

    out = '#ifndef MODS_H_\n'
    out += '#define MODS_H_\n\n'

    for i in l:
        out += '#ifdef MOD_%s\n' % i['url'].upper()
        out += '#define MOD_NAME "%s"\n' % i['name']
        out += '#define MOD_URL "%s"\n' % i['url']
        out += '#endif // MOD_%s\n\n' % i['url']

    out += '#endif'

    f = open("../src/include/mods.h", "w")
    f.write(out)
    f.close()

    f = open("../../hm_gameserver/src/include/mods.h", "w")
    f.write(out)
    f.close()

if __name__ == '__main__':
    prepare_mods()
