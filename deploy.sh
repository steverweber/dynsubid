#!/usr/bin/env bash
set -exu

# Example that returns an offset between 1000000000 and 2000000000
<<'EOF' cat > /usr/local/sbin/nss_dynsubid
#!/usr/bin/bash
# ARGS: $1="-u|-g" $2="id"
# NOTE: max id range is assumed signed int ~2,147,483,647
# im using same range for "subuid -u" and "subgid -g"
# set PATH because we assume no environment
PATH=/usr/bin:$PATH
# sanitize user
id=$(id -u "$2") || exit 9
# advanced offset using hash of id to reduce overlap in range of one billion
offset="1$(printf "%09u" "0x$(echo "$id"|md5sum|head -c16)"|tail -c9)"
echo "$id:$offset:65536"
# to include records from /etc/subuid /etc/subgid
# f=/etc/su${1//-/}bid
# test -e "$f" && {
#     grep "^$id:" "$f"
#     grep "^$(id --name -u "$2"):" "$f" #UNSAFE_REGEX
# }
EOF
chmod +x /usr/local/sbin/nss_dynsubid

# libnss3-dev
apt install -y build-essential libsubid-dev uidmap
gcc -shared -fPIC -o /usr/lib/x86_64-linux-gnu/libsubid_dynsubid.so libsubid_dynsubid.c
# #ln -s /lib/x86_64-linux-gnu/libsubid_subidauto.so.1 /lib/x86_64-linux-gnu/libsubid_subidauto.so

# append to nsswitch if nothing handling subid
# NOTE: only ONE module can be added to subid in nsswitch.conf
grep -q 'subid:' /etc/nsswitch.conf \
    || echo 'subid: dynsubid' >> /etc/nsswitch.conf

# basic test
getsubids root

