#include <hmbase.h>
#include <proto.h>

struct hm_log_s *lg;

#define DUMP(m_dst, m_i, m_s)\
    int i##m_i = 0;\
    printf("dumping [%s]\n", m_s);\
    for(i##m_i = 0; i##m_i < m_i; i##m_i++) {\
        printf("%x", m_dst[i##m_i]);\
    }\
    printf("\n");

#define REPEAT\
    packet_free(p);\
    {\
        for(i = 0; i < 10; i++) {\
            parse_packet(p, buf, nbuf);\
            build_packet_test(p, buf);\
            packet_free(p);\
        }\
    }

#define build_packet_test(m_p, m_buffer)\
    char *ptrtest##m_buffer;\
    ptrtest##m_buffer = m_buffer;\
    n##m_buffer = serialize(m_p, &ptrtest##m_buffer, ptrtest##m_buffer + sizeof(m_buffer));

#define string(m_dst, m_ndst, m_s)\
    m_ndst = strlen(m_s);\
    m_dst = malloc(m_ndst);\
    memcpy(m_dst, m_s, m_ndst);

int main()
{
    struct packet_s *p;
    struct hm_log_s l;
    int i;

    hm_log_open(&l, NULL, LOG_DEBUG);
    lg = &l;

    // chooseoption
    {
        struct chooseoption_s *c;
        c = malloc(sizeof(*c));
        c->id = 0xffff;
        c->index = 0xffff;
        c->target = 0xffff;
        c->suboption = 0xffff;
        c->position = 0xffff;

        build_packet(p, c, P_CHOOSEOPTION, buf);
        REPEAT;
    }

    // turn timer
    {
        struct turntimer_s *c;
        c = malloc(sizeof(*c));
        c->seconds = 0xffff;
        c->turn = 0xffff;
        c->show = 0xff;

        build_packet(p, c, P_TURNTIMER, buf);
        REPEAT;
    }

    // gamesetup
    {
        struct gamesetup_s *c;
        c = malloc(sizeof(*c));
        c->board = 0xffff;
        c->maxsecrets = 0xffff;
        c->maxfriendlyminions = 0xffff;
        c->stuckdisconnect = 0xffff;
        c->keepalive = 0xffff;

        build_packet(p, c, P_TURNTIMER, buf);
        REPEAT;
    }

    // handshake
    {
        struct handshake_s *c;
        c = malloc(sizeof(*c));
        c->gamehandle = 0xffff;
        string(c->password, c->npassword, "password");
        c->clienthandle = 0xffff;
        c->mission = 0xffff;
        string(c->version, c->nversion, "version");

        struct platform_s *pl;
        pl = malloc(sizeof(*pl));
        pl->os = 0xffff;
        pl->screen = 0xffff;
        string(pl->name, pl->nname, "name");
        pl->store = 0xffff;

        c->platform = pl;

        build_packet(p, c, P_HANDSHAKE, buf);
        REPEAT;
    }

    // chooseentities
    {
        struct chooseentities_s *c;
        struct chooseentities_ent_s *e;
        c = malloc(sizeof(*c));
        c->id = 0xffff;
        c->nentity = 4;
        c->entity = NULL;
        for(i = 0; i < c->nentity; i++) {
            e = malloc(sizeof(*e));
            e->entity = i;
            e->next = c->entity;
            c->entity = e;
        }

        build_packet(p, c, P_CHOOSEENTITIES, buf);
        REPEAT;
    }

    // entitychoices
    {
        struct entitychoices_s *c;
        struct entitychoices_entities_s *e;
        c = malloc(sizeof(*c));
        c->id = 0xffff;
        c->type = 0xffff;
        c->countmin = 0xffff;
        c->countmax = 0xffff;
        c->source = 0xffff;
        c->player_id = 0xffff;
        c->entities = NULL;

        for(i = 0; i < 10; i++) {
            e = malloc(sizeof(*e));
            e->entity = i;
            e->next = c->entities;
            c->entities = e;
        }

        build_packet(p, c, P_ENTITYCHOICES, buf);
        REPEAT;
    }

    // userui
    {
        struct userui_s *c;
        struct mouseinfo_s *e;
        c = malloc(sizeof(*c));

        e = malloc(sizeof(*e));
        e->arroworigin = 0xffff;
        e->heldcard = 0xffff;
        e->overcard = 0xffff;
        e->x = 0xffff;
        e->y = 0xffff;

        c->mouseinfo = e;
        c->emote = 0xffff;
        c->player_id = 0xffff;

        build_packet(p, c, P_USERUI, buf);
        REPEAT;
    }

    return 0;
}
