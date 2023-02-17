#ifndef _STIP_MONGO_H_
#define _STIP_MONGO_H_

#include <mongoc/mongoc.h>
#include "sim-event.h"

mongoc_uri_t * stip_mongo_get_uri(const char* uri_string, bson_error_t *error);
mongoc_client_t * stip_mongo_get_client(mongoc_uri_t *uri, bson_error_t *error);
bson_t * stip_mongo_get_alarm_bson(SimEvent *event);
void stip_mongo_insert_alarm(const char *database_name, const char *collection_name, SimEvent *event);
#endif
