#ifndef _STIP_MONGO_H_
#define _STIP_MONGO_H_

#include <mongoc/mongoc.h>
#include "sim-event.h"

mongoc_uri_t * stip_get_mongo_uri(const char* uri_string, bson_error_t *error);
mongoc_client_t * stip_get_mongo_client(mongoc_uri_t *uri, bson_error_t *error);
bson_t * stip_get_alarm_bson(SimEvent *event);
void stip_insert_alarm_into_mongodb(const char *database_name, const char *collection_name, SimEvent *event);
int stip_write_alarm_to_mongodb(gchar* insert_statement, SimEvent *event);
#endif
