#!/usr/bin/env bash
set -exu

# Example that returns an offset between 1000000000 and 2000000000
<<'EOF' cat > /usr/local/sbin/nss_dynsubid
#!/usr/bin/env bash
# ARGS: $1="-u|-g" $2="id"
# NOTE: max id range is ~2,147,483,647
# use same range for subuid -u and subgid -g
# sanitize user
id=$(id -u "$2") || exit 9
##-- basic: offset="$(($id + 1000000000))"
##-- advanced using hash of id to reduce overlap in range of one billion
offset="1$(printf "%09u" "0x$(echo "$id"|md5sum|head -c16)"|tail -c9)"
echo "$id:$offset:65536"
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

