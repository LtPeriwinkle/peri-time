# peri time

inspired by [friend time](https://github.com/KevinNovak/Friend-Time) and with the eventual sole difference that
moderators/admins do not need to ping users to set their time. This was requested specifically by a friend of mine on discord.

### requirements
* sqlite3
* a tzdb installed on your system (probably already there)
* c++11-compliant compiler
* cmake

### usage
to get the executable:

```sh
mkdir build && cd build
cmake ..
make -j3 # or whatever number of jobs you want
```

then create a database with sqlite3:

```sh
sqlite3 tz.db
```

place your bot token (obtain from the discord developer portal) in a file named `.bot-token` in the same folder as the executable and run.
