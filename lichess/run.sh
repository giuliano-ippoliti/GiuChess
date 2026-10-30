#!/bin/sh
# Starts the GiuChess bot on Lichess. Extra arguments go to lichess-bot,
# e.g. "lichess/run.sh -u" to upgrade the account to a BOT account
# (irreversible), "-v" for verbose logs.
set -e
cd "$(dirname "$0")"

if [ ! -d lichess-bot/venv ]; then
	echo "lichess-bot is not installed: run lichess/setup.sh first" >&2
	exit 1
fi
if [ -z "$LICHESS_BOT_TOKEN" ]; then
	echo "Set the LICHESS_BOT_TOKEN environment variable to the bot account's token" >&2
	exit 1
fi
cd lichess-bot
# macOS: keep the Mac awake while the bot runs. A sleeping Mac freezes the
# engine while the Lichess clock keeps running, and loses the game on time.
if command -v caffeinate >/dev/null 2>&1; then
	exec caffeinate -is venv/bin/python lichess-bot.py --config ../config.yml "$@"
fi
exec venv/bin/python lichess-bot.py --config ../config.yml "$@"
