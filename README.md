The job of this tool is to remove the need to manage /etc/subuid and subgid for HPC servers
when they already have shared uid and gid provided by another system like SSSD or LDAP.

testing
-------

    /usr/local/sbin/nss_dynsubid -u $USER
    /usr/local/sbin/nss_dynsubid -g $USER
    getsubids $USER
    getsubids -g $USER


how it works
------------

 - tools like `getsubids` use nss to resolve subids.
 - tools also include podman and apptainer
 - `/etc/nsswitch.conf` supports `subid`
 - the `libsubid_dynsubid.c` program runs `/usr/local/sbin/nss_dynsubid` to do lookups
 - `/usr/local/sbin/nss_dynsubid` can be a basic shell script or whatever... query a DB 


The shell script used in the example `deploy.sh` does some cleaver things that are not 100% safe! Some users could have overlaped ranges.

The problem this targets is when you are running a SLURM HPC setup that supports containers where the user count is large like the population of a University. This also removes the need to sync possibly large /etc/subuid and /etc/subgid accross many compute nodes.


This does not support mutiple ranges and other fancy stuff. If i get burned I'll add it.
