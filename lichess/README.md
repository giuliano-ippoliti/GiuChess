# GiuChess on Lichess

GiuChess plays on [lichess.org](https://lichess.org) as a bot account through
[lichess-bot](https://github.com/lichess-bot-devs/lichess-bot), the official
bridge between the Lichess Bot API and UCI engines. lichess-bot runs on your
machine: it receives the challenges and the moves from Lichess and drives the
engine through UCI. The bot plays only while it runs.

## Setup (once)

1. **Build the engine** at the repository root: `make`.
2. **Install lichess-bot**: `lichess/setup.sh`. It clones lichess-bot into
   `lichess/lichess-bot/` with its own Python virtual environment (Python 3.10
   or later: `brew install python` on macOS), and copies
   `config.yml.example` to `lichess/config.yml`. Both are ignored by git.
3. **Create a new Lichess account for the bot.** It must never have played a
   game: the upgrade to a BOT account fails otherwise. Do not use your
   personal account.
4. **Create an API token** for that account, logged in as the bot, with the
   "Play games with the bot API" (`bot:play`) scope:
   https://lichess.org/account/oauth/token/create?scopes[]=bot:play&description=GiuChess
   Lichess shows it only once: keep it in a password manager.
5. **Give the token to lichess-bot** through an environment variable, never in
   a file of the repository:
   ```bash
   read -s LICHESS_BOT_TOKEN && export LICHESS_BOT_TOKEN   # paste the token, then Enter
   ```
6. **Upgrade the account to a BOT account.** This is **irreversible**: the
   account can then only play through the API.
   ```bash
   lichess/run.sh -u
   ```
   On success the bot starts right away.

## Playing

```bash
export LICHESS_BOT_TOKEN=...   # or the read -s line above
lichess/run.sh
```

The bot then accepts the challenges allowed by `lichess/config.yml`. To play
it, challenge it from another account on its Lichess profile page. Stop it
with Ctrl-C. `lichess/run.sh -v` shows all the communication with Lichess.

## Configuration

`lichess/config.yml` (from `config.yml.example`) overrides lichess-bot's
defaults; the full list of settings is in lichess-bot's `config.yml.default`.
Main choices for GiuChess:

- **One game at a time** (`challenge.concurrency: 1`): the engine uses one CPU
  core per game.
- **Blitz, rapid and classical, base time of at least 1 minute**: bullet is too
  fast for an engine reaching depth 6-7.
- **No pondering, no UCI options**: GiuChess does not support them.
- **No resignation or draw offers**: the bot plays every game to the end.
- **`move_overhead: 1000`**: one second per move kept for network lag;
  increase it if the bot loses on time.
- **`matchmaking.allow_matchmaking: false`**: the bot only answers challenges.
  Set it to `true` to make it challenge other bots when idle.

The engine is started from the repository root (`engine.dir: "../.."`),
without arguments: GiuChess speaks UCI by default.
