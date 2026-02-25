#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "errors.h"
#include "https.h"
#include "keydb.h"

#define HEADER_SIZE 4096
#define BODY_SIZE 16384
#define API_TYPES 1

#define CLAUDE_IDX 0

#define CLAUDE_HOST "api.anthropic.com"
#define CLAUDE_PATH "v1/messages"

static sockres_t* open_conns[API_TYPES] = { NULL };
static const char* system_prompt =
"You are \\\"how\\\", a Unix-style manual page command. Treat the input as if prefixed with \\\"how\\\". Output concise technical documentation including a short description, required steps or syntax, and at least one minimal working example. No conversational language. No questions. No disclaimers. No references to being an AI. No greetings. No filler. Keep output compact but complete enough to execute or implement immediately. Use plain text only. Output strictly in man(7) roff format.Do not use markdown.";

char* claude_get(char* question) {
    sockres_t* sockres = open_conns[CLAUDE_IDX];
    if (sockres == NULL) {
        /* Establish HTTPS connection if an open one doesn't exist */
        sockres = https_connect(CLAUDE_HOST);
        open_conns[CLAUDE_IDX] = sockres;
    }

    char* api_key = keydb_get("CLAUDE");

    char exhdrs[HEADER_SIZE];
    int32_t hdr_len = snprintf(exhdrs, HEADER_SIZE,
        "anthropic-version: 2023-06-01\r\n"
        "x-api-key: %s\r\n",
        api_key
    );

    if (hdr_len <= 0 || hdr_len >= (int32_t) HEADER_SIZE) {
        FATAL(API_FAIL_EC, "Extra headers buffer is too small for Claude!\n");
    }

    char body[BODY_SIZE];
    int32_t body_len = snprintf(body, BODY_SIZE,
        "{"
        "\"model\": \"claude-sonnet-4-20250514\","
        "\"max_tokens\": 1024,"
        "\"system\": \"%s\","
        "\"messages\": [ {\"role\": \"user\", \"content\": \"%s\"} ]"
        "}",
        system_prompt,
        question
    );

    if (body_len <= 0 || body_len >= (int32_t) BODY_SIZE) {
        FATAL(API_FAIL_EC, "Body buffer is too small for Claude!\n");
    }

    https_post(sockres, CLAUDE_HOST, CLAUDE_PATH, exhdrs, body, body_len);
    return NULL;
}
