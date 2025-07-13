# ~/.bashrc: executed by bash(1) for non-login shells.
# see /usr/share/doc/bash/examples/startup-files (in the package bash-doc)
# for examples

# If not running interactively, don't do anything
case $- in
    *i*) ;;
      *) return;;
esac

# don't put duplicate lines or lines starting with space in the history.
# See bash(1) for more options
HISTCONTROL=ignoreboth

# append to the history file, don't overwrite it
shopt -s histappend

# for setting history length see HISTSIZE and HISTFILESIZE in bash(1)
HISTSIZE=1000
HISTFILESIZE=2000

# check the window size after each command and, if necessary,
# update the values of LINES and COLUMNS.
shopt -s checkwinsize

# If set, the pattern "**" used in a pathname expansion context will
# match all files and zero or more directories and subdirectories.
#shopt -s globstar

# make less more friendly for non-text input files, see lesspipe(1)
#[ -x /usr/bin/lesspipe ] && eval "$(SHELL=/bin/sh lesspipe)"

# set variable identifying the chroot you work in (used in the prompt below)
if [ -z "${debian_chroot:-}" ] && [ -r /etc/debian_chroot ]; then
    debian_chroot=$(cat /etc/debian_chroot)
fi

# set a fancy prompt (non-color, unless we know we "want" color)
case "$TERM" in
    xterm-color|*-256color) color_prompt=yes;;
esac

# uncomment for a colored prompt, if the terminal has the capability; turned
# off by default to not distract the user: the focus in a terminal window
# should be on the output of commands, not on the prompt
#force_color_prompt=yes

if [ -n "$force_color_prompt" ]; then
    if [ -x /usr/bin/tput ] && tput setaf 1 >&/dev/null; then
	# We have color support; assume it's compliant with Ecma-48
	# (ISO/IEC-6429). (Lack of such support is extremely rare, and such
	# a case would tend to support setf rather than setaf.)
	color_prompt=yes
    else
	color_prompt=
    fi
fi

if [ "$color_prompt" = yes ]; then
    PS1='${debian_chroot:+($debian_chroot)}\[\033[01;32m\]\u@\h\[\033[00m\]:\[\033[01;34m\]\w\[\033[00m\]\$ '
else
    PS1='${debian_chroot:+($debian_chroot)}\u@\h:\w\$ '
fi
unset color_prompt force_color_prompt

# If this is an xterm set the title to user@host:dir
case "$TERM" in
xterm*|rxvt*)
    PS1="\[\e]0;${debian_chroot:+($debian_chroot)}\u@\h: \w\a\]$PS1"
    ;;
*)
    ;;
esac

# enable color support of ls and also add handy aliases
if [ -x /usr/bin/dircolors ]; then
    test -r ~/.dircolors && eval "$(dircolors -b ~/.dircolors)" || eval "$(dircolors -b)"
    alias ls='ls --color=auto'
    #alias dir='dir --color=auto'
    #alias vdir='vdir --color=auto'

    #alias grep='grep --color=auto'
    #alias fgrep='fgrep --color=auto'
    #alias egrep='egrep --color=auto'
fi

# colored GCC warnings and errors
#export GCC_COLORS='error=01;31:warning=01;35:note=01;36:caret=01;32:locus=01:quote=01'

# some more ls aliases
#alias ll='ls -l'
#alias la='ls -A'
#alias l='ls -CF'

# Alias definitions.
# You may want to put all your additions into a separate file like
# ~/.bash_aliases, instead of adding them here directly.
# See /usr/share/doc/bash-doc/examples in the bash-doc package.

if [ -f ~/.bash_aliases ]; then
    . ~/.bash_aliases
fi

# enable programmable completion features (you don't need to enable
# this, if it's already enabled in /etc/bash.bashrc and /etc/profile
# sources /etc/bash.bashrc).
if ! shopt -oq posix; then
  if [ -f /usr/share/bash-completion/bash_completion ]; then
    . /usr/share/bash-completion/bash_completion
  elif [ -f /etc/bash_completion ]; then
    . /etc/bash_completion
  fi
fi
# Added for Claude Code and proxy functionality
export PATH="/usr/local/bin:$PATH"

# Source user-level API keys if file exists
if [ -f ~/.env ]; then
    source ~/.env
fi

# Node options for specific tools
export NODE_OPTIONS="--experimental-global-webcrypto"

# Set Claude CLI to use the local proxy
export ANTHROPIC_BASE_URL="http://localhost:8082"

# Wrapper for codex CLI to always use Gemini provider and a default model
codex() {
  # Ensure GEMINI_API_KEY is available for the command environment
  command codex --provider gemini --model gemini-1.5-pro-latest "$@"
}

# Helper for git clone to auto-convert https to ssh for github
gitclone() {
  url=$1
  if [[ $url == https://github.com/* ]]; then
    ssh_url=$(echo $url | sed "s|https://github.com/|git@github.com:|")
    echo "Converting to SSH URL: $ssh_url"
    git clone "$ssh_url" "${@:2}"
  else
    git clone "$@"
  fi
}

# --- API Proxy Management Functions ---
start_claude_proxy() {
  echo "Starting API Proxy server (gemini-code)..."
  cd /opt/gemini-code
  # Ensure .env is present in /opt/gemini-code for the server to pick up
  if [ ! -f .env ]; then
    echo "Warning: /opt/gemini-code/.env not found. Proxy might use defaults or fail."
    echo "You may need to create it from .env.example and populate GEMINI_API_KEY."
  fi
  nohup /opt/venv/bin/python -m uvicorn server:app --host 0.0.0.0 --port 8082 > /tmp/proxy.log 2>&1 &
  sleep 2 # Give it a moment to start
  echo "Proxy server started. Check status with 'check_proxy_health'."
}

check_proxy_health() {
  echo "Checking API Proxy health..."
  # Try both /health and / (root) as health check endpoints
  if curl -sf http://localhost:8082/health > /dev/null 2>&1 || curl -sf http://localhost:8082/ > /dev/null 2>&1; then
    echo "✅ API Proxy is running."
  else
    echo "❌ API Proxy is NOT running or not responding."
    echo "   Last logs from /tmp/proxy.log:"
    tail -n 20 /tmp/proxy.log
    echo "   To restart, try: stop_claude_proxy && start_claude_proxy"
  fi
}

stop_claude_proxy() {
  echo "Stopping API Proxy server..."
  pkill -f "uvicorn server:app --host 0.0.0.0 --port 8082"
  sleep 1
  if ! pgrep -f "uvicorn server:app --host 0.0.0.0 --port 8082" > /dev/null; then
    echo "API Proxy stopped."
  else
    echo "API Proxy may still be running. Check processes or try 'pkill -9 -f "uvicorn server:app"'."
  fi
}

# Function to display current relevant environment variables
show_env() {
  echo "--- Key Environment Variables ---"
  echo "GEMINI_API_KEY: ${GEMINI_API_KEY:0:5}... (${#GEMINI_API_KEY} chars)"
  echo "OPENAI_API_KEY: ${OPENAI_API_KEY:0:5}... (${#OPENAI_API_KEY} chars)"
  echo "ANTHROPIC_API_KEY: ${ANTHROPIC_API_KEY:0:5}... (${#ANTHROPIC_API_KEY} chars)"
  echo "ANTHROPIC_BASE_URL: ${ANTHROPIC_BASE_URL}"
  echo "Proxy .env file: /opt/gemini-code/.env"
  if [ -f /opt/gemini-code/.env ]; then
      echo "Contents of /opt/gemini-code/.env (API keys masked):"
      grep -v "API_KEY" /opt/gemini-code/.env | sed 's/API_KEY=.*/API_KEY=****/g'
      grep "GEMINI_API_KEY" /opt/gemini-code/.env | awk -F= '{print $1 "=" substr($2,1,5) "..."}'
  fi
  echo "-------------------------------"
}
