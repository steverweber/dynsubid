#include <nss.h>
#include <stdlib.h>
#include <string.h>
#include <shadow/subid.h>
#include <stdio.h>

// MIT

// https://fossies.org/linux/shadow/libsubid/subid.h.in
// partial implementation of `subid_nss_ops`
// https://github.com/shadow-maint/shadow/blob/d4b6d1549b2af48ce3cb6ff78d9892095fb8fdd9/lib/prototypes.h#L271

// TODO: allow mutiple range_count.
bool _dynsubid_cmd(const char* id, enum subid_type id_type, int *start, int *count) {
    char command[512];
    if (id_type == ID_TYPE_UID) {
        //printf("ID_TYPE_UID\n");
        snprintf(command, sizeof(command), "/usr/local/sbin/nss_dynsubid -u %s", id);
    }
    if (id_type == ID_TYPE_GID) {
        //printf("ID_TYPE_GID\n");
        snprintf(command, sizeof(command), "/usr/local/sbin/nss_dynsubid -g %s", id);
    }
    //printf("DEBUG: command %s\n", command);
    FILE *fp = popen(command, "r");
    if (!fp) {
        return false;
    }
    char line[512];
    // parse output line assumed format: id:start:count
    // NOTE: we ignore id
    if (fgets(line, sizeof(line), fp) != NULL) {
        //printf("DEBUG: output %s\n", line);
        char *token = strtok(line, ":");
        if (!token) { pclose(fp); return false; }
        //printf("DEBUG: id %s\n", token);
        //id = strdup(token);
        token = strtok(NULL, ":");
        if (!token) { pclose(fp); return false; }
        //printf("DEBUG: start %s\n", token);
        *start = atoi(token);
        token = strtok(NULL, ":");
        if (!token) { pclose(fp); return false; }
        //printf("DEBUG: count %s\n", token);
        *count = atoi(token);
        pclose(fp);
    } else {
        pclose(fp);
        return false;
    }
    if (*start == 0 || *count == 0) {
        // assume an issue using atoi to convert to int
        return false;
    }
    return true;
}


/* Find all subid ranges delegated to a user.
 * NOTE: we only return one range!
 * Usage in shadow-utils: libsubid: get_sub?id_ranges() -> list_owner_ranges()
 *
 * SUBID_RANGES Reply:
 * 0-3: 32bit unsigned number of UID results
 * 4-7: 32bit unsigned number of GID results
 * For each result (sub-uid ranges first):
 * 0-3: 32bit number with "start" id
 * 4-7: 32bit number with "count" (range size)
 */
enum subid_status shadow_subid_list_owner_ranges(
    const char *id, enum subid_type id_type, struct subid_range **ranges, int *range_count)
{
    // carefull steve.. this likly runs as root and accepts userspace data.
    if ( !id || ((id_type != ID_TYPE_UID) && (id_type != ID_TYPE_GID)) || !ranges || !range_count ) {
        // unset arg or unknowen idtype
        return SUBID_STATUS_ERROR;
    }
    // lets only allow one range_count.
    uint32_t start = 0;
    uint32_t count = 0;
    bool r = _dynsubid_cmd(id, id_type, &start, &count);
    if (!r) {
        return SUBID_STATUS_ERROR;
    }
    *range_count = 1;
    *ranges = malloc(1 * sizeof(struct subid_range));
    if (!*ranges) {
        return SUBID_STATUS_ERROR;
    }
    (*ranges)[0].start = start;
    (*ranges)[0].count = count;
    return SUBID_STATUS_SUCCESS;
}

// Does a user own a given subid range?
// in shadow-utils: newuidmap/user busy : have_sub_uids() -> has_range()
enum subid_status shadow_subid_has_range(
    const char *id, unsigned long start, unsigned long count, enum subid_type id_type, bool *result)
{
    unsigned long end = start + count;
    if (!result || (end < start)) {
        return SUBID_STATUS_ERROR;
    }
    if (count == 0) {
        *result = true;
        return SUBID_STATUS_SUCCESS;
    }
    uint32_t s = 0;
    uint32_t c = 0;
    bool r = _dynsubid_cmd(id, id_type, &s, &c);
    if (!r) {
        *result = false;
        return SUBID_STATUS_SUCCESS;
    }
    if ((start < s) || ((s + c) > end)) {
        *result = false;
        return SUBID_STATUS_SUCCESS;
    }
    *result = true;
    return SUBID_STATUS_SUCCESS;
}

// Find uids who own a given subid.
// Usage in shadow-utils: libsubid: get_sub?id_owners() -> find_subid_owners()
enum subid_status shadow_subid_find_subid_owners(
    unsigned long subid, enum subid_type id_type, uid_t **uids,int *count)
{
    // not implemented.
    return SUBID_STATUS_ERROR;
}

// Release memory used in shadow_subid_*()
void shadow_subid_free(void *ptr) {
    free(ptr);
}
