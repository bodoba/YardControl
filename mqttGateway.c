/* *********************************************************************************** */
/*  Copyright (c) 2018 by Bodo Bauer <bb@bb-zone.com>                                  */
/*                                                                                     */
/*  This program is free software: you can redistribute it and/or modify               */
/*  it under the terms of the GNU General Public License as published by               */
/*  the Free Software Foundation, either version 3 of the License, or                  */
/*  (at your option) any later version.                                                */
/*                                                                                     */
/*  This program is distributed in the hope that it will be useful,                    */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of                     */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                      */
/*  GNU General Public License for more details.                                       */
/*                                                                                     */
/*  You should have received a copy of the GNU General Public License                  */
/*  along with this program.  If not, see <http://www.gnu.org/licenses/>.              */
/* *********************************************************************************** */
#include <mosquitto.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "mqttGateway.h"
#include "logging.h"

/* ----------------------------------------------------------------------------------- *
 * Handle to broker
 * ----------------------------------------------------------------------------------- */
static struct mosquitto *mosq = NULL;
static        mqttIncoming_t *subscriptionList = NULL;


mosquitto_message_callback_set(
                               struct     mosquitto     *    mosq,
                               void         (*on_message)(struct mosquitto *, void *, const struct mosquitto_message *)
                               )


/* ----------------------------------------------------------------------------------- *
 * Connect to MQTT broker
 * ----------------------------------------------------------------------------------- */
bool mqttInit( const char* broker, int port, int keepalive, mqttIncoming_t *subscriptions) {
    bool success = true;
    int err;
    
    mosquitto_lib_init();
    mosq = mosquitto_new(NULL, true, NULL);
    if(mosq){
        err = mosquitto_connect(mosq, broker, port, keepalive);
        if( err != MOSQ_ERR_SUCCESS ) {
            writeLog(LOG_ERR, "Error: mosquitto_connect [%s]\n", mosquitto_strerror(err));
            success = false;
        }
    } else {
        writeLog(LOG_ERR, "Error: Out of memory.\n");
        success = false;
    }
    
    err = mosquitto_loop_start(mosq);
    if( err != MOSQ_ERR_SUCCESS ) {
        writeLog(LOG_ERR, "Error: mosquitto_connect [%s]\n", mosquitto_strerror(err));
        success = false;
    } else {
        subscriptionList = subscriptions;
        int idx = 0;
        while (subscriptionList[idx].topic) {
            writeLog(LOG_INFO, "Supscribe to MQTT topic: %s", subscriptionList[idx].topic);
            mosquitto_subscribe( mosq, NULL, subscriptionList[idx].topic, 0);
            idx++;
        }
    }
    return success;
}

/* ----------------------------------------------------------------------------------- *
 * End MQTT broker connection
 * ----------------------------------------------------------------------------------- */
void mqttEnd( void ) {
    mosquitto_loop_stop(mosq, true);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    mosq = NULL;
}

/* ----------------------------------------------------------------------------------- *
 * Publish MQTT message
 * ----------------------------------------------------------------------------------- */
bool mqttPublish ( const char *topic, const char *message ) {
    bool success = true;
    int  err;
    
    if ( mosq ) {
        err = mosquitto_publish( mosq, NULL, topic, strlen(message), message, 0, false);
        if ( err != MOSQ_ERR_SUCCESS) {
            fprintf(stderr, "Error: mosquitto_publish failed [%s]\n", mosquitto_strerror(err));
            success = false;
        }
        mosquitto_loop(mosq, 0, 1);
    } else {
        fprintf(stderr, "Error: mosq == NULL, Init failed?\n");
        success = false;
    }
    return success;
}
