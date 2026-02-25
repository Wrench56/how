#define _XOPEN_SOURCE 500

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "yyjson.h"

#include "errors.h"
#include "https.h"
#include "keydb.h"

#define HEADER_SIZE 4096
#define BODY_SIZE 16384

#define CLAUDE_HOST "api.anthropic.com"
#define CLAUDE_PATH "v1/messages"

static const char* default_viewer = "man -l -";
static const char* default_system_prompt =
"You are \\\"how\\\", a Unix-style manual page command. Treat the input as if prefixed with \\\"how\\\". Output concise technical documentation including a short description, required steps or syntax, and at least one minimal working example. No conversational language. No questions. No disclaimers. No references to being an AI. No greetings. No filler. Keep output compact but complete enough to execute or implement immediately. Use plain text only. Output strictly in man(7) roff format.Do not use markdown.";

static void open_in_viewer(char* viewer_command, const char* text, size_t len) {
    FILE* fp = popen(viewer_command, "w");
    if (fp == NULL) {
        FATAL(API_FAIL_EC, "popen(viewer) failed!\n");
    }

    if (fwrite(text, 1, len, fp) != len) {
        if (ferror(fp) && errno != EPIPE) {
            pclose(fp);
            FATAL(API_FAIL_EC, "Error while writing to viewer!\n");
        }
    }

    int32_t rc = pclose(fp);
    if (rc != 0 && errno != EPIPE) {
        FATAL(API_FAIL_EC, "Viewer exited with nonzero return code\n");
    }
}

static char* anthropic_parse_out(postresp_t* sockresp, size_t* len) {
    yyjson_doc* doc = yyjson_read(sockresp->body, sockresp->body_length, 0);
    if (doc == NULL) {
        *len = 0;
        return NULL;
    }

    yyjson_val* root = yyjson_doc_get_root(doc);

    yyjson_val* content_arr = yyjson_obj_get(root, "content");
    if (content_arr == NULL || !yyjson_is_arr(content_arr)) {
        goto error;
    }

    yyjson_val* first = yyjson_arr_get(content_arr, 0);
    if (first == NULL) {
        goto error;
    }

    yyjson_val* text = yyjson_obj_get(first, "text");
    if (text == NULL || !yyjson_is_str(text)) {
        goto error;
    }

    size_t text_len = yyjson_get_len(text);
    char* out = malloc(text_len + 1);
    if (out == NULL) {
        yyjson_doc_free(doc);
        FATAL(MALLOC_FAIL_EC, "malloc() failed when allocating buffer for returned content!\n");
    }
    memcpy(out, yyjson_get_str(text), text_len);
    out[text_len] = '\0';

    yyjson_doc_free(doc);
    *len = text_len;
    return out;

error:
    yyjson_doc_free(doc);
    *len = 0;
    return NULL;
}

char* claude_get(char* question) {
    sockres_t* sockres = https_connect(CLAUDE_HOST);
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

    char* system_prompt = getenv("HOW_SYSTEM_PROMPT");
    if (system_prompt == NULL) {
        system_prompt = (char*) default_system_prompt;
    }

    yyjson_mut_doc* doc = yyjson_mut_doc_new(NULL);
    yyjson_mut_val* root = yyjson_mut_obj(doc);
    yyjson_mut_doc_set_root(doc, root);

    yyjson_mut_obj_add_str(doc, root, "model", "claude-sonnet-4-20250514");
    yyjson_mut_obj_add_int(doc, root, "max_tokens", 1024);
    yyjson_mut_obj_add_str(doc, root, "system", system_prompt);

    yyjson_mut_val* messages = yyjson_mut_arr(doc);
    yyjson_mut_obj_add_val(doc, root, "messages", messages);

    yyjson_mut_val* msg = yyjson_mut_obj(doc);
    yyjson_mut_obj_add_str(doc, msg, "role", "user");
    yyjson_mut_obj_add_str(doc, msg, "content", question);

    yyjson_mut_arr_add_val(messages, msg);

    size_t json_len;
    char* json = yyjson_mut_write(doc, 0, &json_len);
    if (json == NULL) {
        yyjson_mut_doc_free(doc);
        FATAL(API_FAIL_EC, "yyjson_mut_write() returned NULL!\n");
    }

    postresp_t resp = { 0 };
    https_post(sockres, CLAUDE_HOST, CLAUDE_PATH, exhdrs, json, json_len, &resp);

    yyjson_mut_doc_free(doc);
    free(json);

    size_t len = 0;
    char* text = anthropic_parse_out(&resp, &len);
    char* viewer_command = getenv("HOW_VIEWER_CMD");
    if (viewer_command == NULL) {
        viewer_command = (char*) default_viewer;
    }

    if (text == NULL) {
        FATAL(API_FAIL_EC, "Failed to parse Anthropic response JSON!\n");
    }
    open_in_viewer(viewer_command, text, len);

    free(text);
    free(resp.body);
    return NULL;
}
