# ~/.profile: Executed by Bourne-compatible login SHells.
#
# Path to personal scripts and executables (~/.local/bin).
#
clear
if [ -d "$HOME/.local/bin" ] ; then
PATH=$HOME/.local/bin:$PATH
export PATH
fi

ONDEMAND="$(cat /opt/.tce_dir)"/ondemand
if [ -d "$ONDEMAND" ]; then
PATH=$PATH:"$ONDEMAND"
export PATH
fi


# Environnement variables and prompt for Ash SHell
# or Bash. Default is a classic prompt.
#
PS1='\u@\h:\w\$ '
PAGER='less -EM'
FILEMGR=fluff
EDITOR=editor
MANPAGER='less -isR'

export PS1 PAGER FILEMGR EDITOR MANPAGER

export BACKUP=1 && echo "$BACKUP" | sudo tee /etc/sysconfig/backup >/dev/null
export FLWM_TITLEBAR_COLOR="B0:C4:DE"

if [ -f "$HOME/.ashrc" ]; then
   export ENV="$HOME/.ashrc"
   . "$HOME/.ashrc"
fi

[ ! -f /etc/sysconfig/Xserver ] ||
[ -f /etc/sysconfig/text ] ||
[ -e /tmp/.X11-unix/X0 ] || 
startx
