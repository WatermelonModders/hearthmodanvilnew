# Hearthmod Anvil
Hearthmod anvil is a software stack that allows you to Mod Hearthstone. If you intend to run it, it's recommended that you run a linux OS, preferably debian or ubuntu distribution.

## Components
[hm_lobbyserver](https://github.com/WatermelonModders/hm_lobbyserver) - hearthmod lobby server

[hm_gameserver](https://github.com/WatermelonModders/hm_gameserver) - hearthmod game server

[hm_base](https://github.com/WatermelonModders/hm_base) - hearthmod base library

[hm_client](https://github.com/WatermelonModders/hm_client) - hearthmod client

[hm_database](https://github.com/WatermelonModders/hm_database) - hearthmod latest database snapshot

[hm_sunwell](https://github.com/WatermelonModders/hm_sunwell) - hearthsim custom card generation

[hm_stud](https://github.com/WatermelonModders/hm_stud) - tls un/wrapper

[hm_nginx](https://github.com/WatermelonModders/hm_nginx) - nginx web server

[hm_web](https://github.com/WatermelonModders/hm_web) - hearthmod web interface

## Guide

Clone hearthmod software stack

```sh
git clone https://github.com/WatermelonModders/hearthmodanvil.git
```

Run 

```sh
cd hearthmod/
bash host_ctl_ubuntu.sh uninstalled
```

which compiles and installs the entire hearthmod stack. Also, it doesn't check for you local couchbase, nginx or stud instances. If you don't want to mess them, consider using virtual environment or modify installation script. If you don't run ubuntu or debian, please consider installation script alteration that suits your distro and creating an upstream pull request. Script host_ctl_ubuntu.sh was only tested on Ububtu 16.04.

Once compiled, run:
```sh
bash host_ctl_ubuntu.sh start [ip]
``` 
where IP is address of your gameserver (your current IP in most cases) and you can play.

Then run clients:
```sh
hearthmod/hs_client1/hearthmod_client/linux
```
and
```sh
hearthmod/hs_client2/hearthmod_client/linux
