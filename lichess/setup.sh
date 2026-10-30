#!/bin/sh
# Installs lichess-bot in lichess/lichess-bot with its own Python virtual
# environment, and creates lichess/config.yml from the example.
# lichess-bot needs Python 3.10 or later.
set -e
cd "$(dirname "$0")"

for p in python3.14 python3.13 python3.12 python3.11 python3.10 python3; do
	if command -v "$p" >/dev/null 2>&1 &&
	   "$p" -c 'import sys; sys.exit(sys.version_info < (3, 10))' 2>/dev/null; then
		PYTHON=$p
		break
	fi
done
if [ -z "$PYTHON" ]; then
	echo "lichess-bot needs Python 3.10 or later (e.g. brew install python)" >&2
	exit 1
fi

if [ ! -d lichess-bot ]; then
	git clone --depth 1 https://github.com/lichess-bot-devs/lichess-bot.git
fi
if [ ! -d lichess-bot/venv ]; then
	"$PYTHON" -m venv lichess-bot/venv
fi
lichess-bot/venv/bin/python -m pip install --quiet -r lichess-bot/requirements.txt

if [ ! -f config.yml ]; then
	cp config.yml.example config.yml
	echo "Created lichess/config.yml"
fi
if [ ! -x ../GiuChess-2.0 ]; then
	echo "Build the engine before starting the bot: make" >&2
fi
echo "Done. Set LICHESS_BOT_TOKEN, then start the bot with lichess/run.sh"
