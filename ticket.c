#include <stdlib.h>
#include "post.h"
#include "rtclient/ticket.h"

typedef struct rtclient_ticket rtclient_ticket;
typedef struct rtclient_ticket_history_list rtclient_ticket_history_list;

void rtclient_ticket_new(const char *queue
			, const char *requestor
			, const char *subject
			, const char *cc
			, const char *admin_cc
			, const char *owner
			, const char *status
			, const char *priority
			, const char *initial_priority
			, const char *final_priority
			, const char *time_estimated
			, const char *starts
			, const char *due
			, const char *text)
{
	post("REST/1.0/ticket/new", (const char *[]){
			"ticket/new", "id"
			, queue, "Queue"
			, requestor, "Requestor"
			, subject, "Subject"
			, cc, "Cc"
			, admin_cc, "AdminCc"
			, owner, "Owner"
			, status, "Status"
			, priority, "Priority"
			, initial_priority, "InitialPriority"
			, final_priority, "FinalPriority"
			, time_estimated, "TimeEstimated"
			, starts, "Starts"
			, due, "Due"
			, text, "Text"
			}, 28);
}

static size_t history_handler(void *contents, size_t size, size_t nmemb
		, void *writedata)
{
	size_t realsize = size * nmemb;
	char response[realsize + 1];
	memcpy(response, contents, realsize);
	response[realsize] = '\0';
	char histories[strlen(response) + 1];
	strcpy(histories, response);
	rtclient_ticket_history_list **listptr
		= (rtclient_ticket_history_list **)writedata;

	char *linesaveptr = NULL;
	char *line = strtok_r(response, "\n", &linesaveptr);
	if (strstr(line, "200 Ok")) {
		line = strtok_r(NULL, "\n", &linesaveptr);
		char *hashsaveptr = NULL;
		char *hash = strtok_r(line, " ", &hashsaveptr);
		hash = strtok_r(NULL, " ", &hashsaveptr);
		char *length = strtok(hash, "/");
		(*listptr)->length = atoi(length);

		rtclient_ticket_history_list *ptr = realloc(*listptr
				, sizeof(*listptr)
				+ (*listptr)->length
				* sizeof(struct rtclient_ticket_history));
		*listptr = ptr;
		rtclient_ticket_history_list *list = *listptr;

		char *historysaveptr = NULL;
		char *history = strtok_r(histories, "#", &historysaveptr);
		history = strtok_r(NULL, "#", &historysaveptr);
		char *linesaveptr = NULL, *line = NULL;
		char *tokensaveptr = NULL, *token = NULL;
		for (size_t i = 0; i < list->length; i++) {
			list->histories[i]
				= malloc(sizeof(struct rtclient_ticket_history));
			struct rtclient_ticket_history *ticket_history
				= list->histories[i];
			line = strtok_r(history, "\n", &linesaveptr);
			line = strtok_r(NULL, "\n", &linesaveptr);
			do {
				token = strtok_r(line, ":", &tokensaveptr);
				if (!strcmp(token, "id")) {
					token = strtok_r(NULL, ":", &tokensaveptr);
					ticket_history->id = atoi(++token);
#ifdef DEBUG
					printf("ID: %u\n", ticket_history->id);
#endif
				} else if (!strcmp(token, "Ticket")) {
					token = strtok_r(NULL, ":", &tokensaveptr);
					ticket_history->ticket = atoi(++token);
#ifdef DEBUG
					printf("Ticket: %u\n", ticket_history->ticket);
#endif
				} else if (!strcmp(token, "TimeTaken")) {
					token = strtok_r(NULL, ":", &tokensaveptr);
					ticket_history->time_taken = atoi(++token);
#ifdef DEBUG
					printf("Time Taken: %u\n", ticket_history->time_taken);
#endif
				} else if (!strcmp(token, "Type")) {
					token = strtok_r(NULL, ":", &tokensaveptr);
					if (!strcmp(++token, "Create"))
						ticket_history->type
							= RTCLIENT_TICKET_HISTORY_TYPE_CREATE;
					else if (!strcmp(token, "EmailRecord"))
						ticket_history->type
							= RTCLIENT_TICKET_HISTORY_TYPE_EMAIL_RECORD;
					else if (!strcmp(token, "Set"))
						ticket_history->type
							= RTCLIENT_TICKET_HISTORY_TYPE_SET;
					else if (!strcmp(token, "SetWatcher"))
						ticket_history->type
							= RTCLIENT_TICKET_HISTORY_TYPE_SET_WATCHER;
					else if (!strcmp(token, "Status"))
						ticket_history->type
							= RTCLIENT_TICKET_HISTORY_TYPE_STATUS;
					else
						ticket_history->type
							= RTCLIENT_TICKET_HISTORY_TYPE_UNKNOWN;
#ifdef DEBUG
					printf("Type: %u\n", ticket_history->type);
#endif
				} else if (!strcmp(token, "Field")) {
					token = strtok_r(NULL, ":", &tokensaveptr);
					if (!strcmp(++token, "Priority"))
						ticket_history->field
							= RTCLIENT_TICKET_HISTORY_FIELD_PRIORITY;
					else if (!strcmp(token, "Status"))
						ticket_history->field
							= RTCLIENT_TICKET_HISTORY_FIELD_STATUS;
					else if (!strcmp(token, "Owner"))
						ticket_history->field
							= RTCLIENT_TICKET_HISTORY_FIELD_OWNER;
					else
						ticket_history->field
							= RTCLIENT_TICKET_HISTORY_FIELD_NONE;
#ifdef DEBUG
					printf("Field: %u\n", ticket_history->field);
#endif
				} else if (!strcmp(token, "OldValue")) {
					token = strtok_r(NULL, ":", &tokensaveptr);
					if (token && strcmp(token, "")) {
						ticket_history->old_value
							= malloc(strlen(token));
						strcpy(ticket_history->old_value
								, ++token);
#ifdef DEBUG
						printf("Old Value: %s\n", ticket_history->old_value);
#endif
					}
				} else if (!strcmp(token, "NewValue")) {
					token = strtok_r(NULL, ":", &tokensaveptr);
					if (token && strcmp(token, "")) {
						ticket_history->new_value
							= malloc(strlen(token));
						strcpy(ticket_history->new_value
								, ++token);
#ifdef DEBUG
						printf("New Value: %s\n", ticket_history->new_value);
#endif
					}
				} else if (!strcmp(token, "Data")) {
					token = strtok_r(NULL, ":", &tokensaveptr);
					ticket_history->data
						= malloc(strlen(token));
					strcpy(ticket_history->data, ++token);
#ifdef DEBUG
					printf("Data: %s\n", ticket_history->data);
#endif
				} else if (!strcmp(token, "Description")) {
					token = strtok_r(NULL, ":", &tokensaveptr);
					ticket_history->description
						= malloc(strlen(token));
					strcpy(ticket_history->description
							, ++token);
#ifdef DEBUG
					printf("Description: %s\n", ticket_history->description);
#endif
				} else if (!strcmp(token, "Content")) {
					token = strtok_r(NULL, ":", &tokensaveptr);
					ticket_history->content
						= malloc(strlen(token));
					strcpy(ticket_history->content, ++token);
#ifdef DEBUG
					printf("Content: %s\n", token);
#endif
				} else if (!strcmp(token, "Creator")) {
					token = strtok_r(NULL, ":", &tokensaveptr);
					ticket_history->creator
						= malloc(strlen(token));
					strcpy(ticket_history->creator, ++token);
#ifdef DEBUG
					printf("Creator: %s\n", ticket_history->creator);
#endif
				} else if (!strcmp(token, "Created")) {
					token = strtok_r(NULL, ":", &tokensaveptr);
#ifdef DEBUG
					printf("Created Token: %s\n", token);
#endif
					/*
					ticket_history->created
						= malloc(sizeof(struct tm));
						*/
				} else if (!strcmp(token, "Attachments")) {
					token = strtok_r(NULL, ":", &tokensaveptr);
#ifdef DEBUG
					printf("Attachments: %s\n", token);
#endif
					break;
				}
			}  while ((line = strtok_r(NULL, "\n", &linesaveptr)));

			history = strtok_r(NULL, "#", &historysaveptr);
		}
	} else {
		free(*listptr);
		*listptr = NULL;
#ifdef DEBUG
#ifdef ANDROID
		__android_log_print(ANDROID_LOG_INFO, "librtclient"
				, "%s response status:\n%s", __func__, line);
#else
		printf("%s response status:\n%s\n", __func__, line);
#endif // ANDROID
#endif // DEBUG
	}

	return realsize;
}

void rtclient_ticket_history(rtclient_ticket_history_list **listptr
		, unsigned int id)
{
	*listptr = malloc(sizeof(rtclient_ticket_history_list));
	(*listptr)->length = 0;
	request(history_handler, (void *)listptr, NULL, "%s%u%s"
			, "REST/1.0/ticket/", id, "/history?format=l");
}
