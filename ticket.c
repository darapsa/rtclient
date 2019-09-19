#include <stdlib.h>
#include "post.h"
#include "rtclient/ticket.h"

typedef struct rtclient_ticketlist rtclient_ticketlist;

void rtclient_ticket_new(const char *queue
			, const char *requestor
			, const char *subject
			, const char *cc
			, const char *admincc
			, const char *owner
			, const char *status
			, const char *priority
			, const char *initialpriority
			, const char *finalpriority
			, const char *timeestimated
			, const char *starts
			, const char *due
			, const char *text)
{
	post("/REST/1.0/ticket/new", (const char *[]){
			"ticket/new", "id"
			, queue, "Queue"
			, requestor, "Requestor"
			, subject, "Subject"
			, cc, "Cc"
			, admincc, "AdminCc"
			, owner, "Owner"
			, status, "Status"
			, priority, "Priority"
			, initialpriority, "InitialPriority"
			, finalpriority, "FinalPriority"
			, timeestimated, "TimeEstimated"
			, starts, "Starts"
			, due, "Due"
			, text, "Text"
			}, 28);
}

static size_t search_callback(void *contents, size_t size, size_t nmemb
		, void *writedata)
{
	size_t realsize = size * nmemb;
	char response[realsize + 1];
	memcpy(response, contents, realsize);
	response[realsize] = '\0';
	char lines[strlen(response) + 1];
	strcpy(lines, response);
	rtclient_ticketlist **listptr = (rtclient_ticketlist **)writedata;

	char *line = strtok(response, "\n");
	if (strstr(line, "200 Ok")) {
		(*listptr)->length = 0;
		line = strtok(NULL, "\n");
		do {
			(*listptr)->length++;
			if (!strcmp(line, "No matching results.")) {
				free(*listptr);
				*listptr = NULL;
				return realsize;
			}
		} while ((line = strtok(NULL, "\n")));
		rtclient_ticketlist *ptr = realloc(*listptr, sizeof(*listptr)
				+ (*listptr)->length * sizeof(char *));
		*listptr = ptr;
		rtclient_ticketlist *list = *listptr;
		char *linesaveptr = NULL;
		line = strtok_r(lines, "\n", &linesaveptr);
		line = strtok_r(NULL, "\n", &linesaveptr);
		char *tokensaveptr = NULL, *token = NULL;
		for (unsigned int i = 0; i < list->length; i++) {
			token = strtok_r(line, ":", &tokensaveptr);
			token = strtok_r(NULL, ":", &tokensaveptr);
			list->tickets[i] = malloc(strlen(token));
			strcpy(list->tickets[i], ++token);
			line = strtok_r(NULL, "\n", &linesaveptr);
		}
	} else {
		free(*listptr);
		*listptr = NULL;
#ifdef DEBUG
		fprintf(stderr, "%s response status: %s\n", __func__, line);
#endif
	}

	return realsize;
}

void rtclient_ticket_search(rtclient_ticketlist **listptr, const char *query)
{
	*listptr = malloc(sizeof(rtclient_ticketlist));
	request("/REST/1.0/search/ticket?query=", query, search_callback
			, (void *)listptr, NULL);
}

void rtclient_ticket_freelist(rtclient_ticketlist *list)
{
	for (unsigned short i = 0; i < list->length; i++)
		free(list->tickets[i]);
	free(list);
	list = NULL;
}
