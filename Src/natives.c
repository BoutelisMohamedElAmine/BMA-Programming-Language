// src/natives.c — Complete natives library
#include "natives.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <curl/curl.h>
#include "cJSON.h"

static char* my_strdup(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char* p = malloc(len + 1);
    memcpy(p, s, len + 1);
    return p;
}

typedef struct { char *data; size_t size; } CurlBuf;

static size_t curl_write(void *ptr, size_t size, size_t nmemb, void *userdata) {
    CurlBuf *buf = (CurlBuf*)userdata;
    size_t new_len = buf->size + size * nmemb;
    buf->data = realloc(buf->data, new_len + 1);
    memcpy(buf->data + buf->size, ptr, size * nmemb);
    buf->data[new_len] = 0;
    buf->size = new_len;
    return size * nmemb;
}

static cJSON* json_find(cJSON *root, const char *path) {
    cJSON *current = root;
    const char *p = path;
    while (*p && current) {
        const char *key_start = p;
        while (*p && *p != '.' && *p != '[') p++;
        int key_len = (int)(p - key_start);
        if (key_len > 0) {
            char key[256];
            if (key_len >= 256) key_len = 255;
            memcpy(key, key_start, key_len);
            key[key_len] = 0;
            current = cJSON_GetObjectItemCaseSensitive(current, key);
            if (!current) return NULL;
        }
        if (*p == '[') {
            p++;
            int index = 0;
            while (*p >= '0' && *p <= '9') { index = index * 10 + (*p - '0'); p++; }
            if (*p == ']') p++;
            if (!cJSON_IsArray(current)) return NULL;
            current = cJSON_GetArrayItem(current, index);
            if (!current) return NULL;
        }
        if (*p == '.') p++;
    }
    return current;
}

Value native_call(const char* name, Value* args, int argc, bool* found) {
    *found = true;

    // ═══════════════════════════════════════════════════════════
    // I/O
    // ═══════════════════════════════════════════════════════════
    if (strcmp(name, "write") == 0) {
        for (int i = 0; i < argc; i++) val_print(args[i]);
        return val_null();
    }
    if (strcmp(name, "writeline") == 0) {
        for (int i = 0; i < argc; i++) val_print(args[i]);
        printf("\n");
        return val_null();
    }

    // ═══════════════════════════════════════════════════════════
    // MATH
    // ═══════════════════════════════════════════════════════════
    if (strcmp(name, "pi") == 0) return val_real(3.14159265358979323846);
    if (strcmp(name, "e") == 0)  return val_real(2.71828182845904523536);

    if (strcmp(name, "sqrt") == 0) {
        if (argc != 1) bma_error("sqrt requires 1 argument");
        double d = (args[0].type == V_INT) ? (double)args[0].i : args[0].d;
        return val_real(sqrt(d));
    }
    if (strcmp(name, "abs") == 0) {
        if (argc != 1) bma_error("abs requires 1 argument");
        if (args[0].type == V_INT) return val_int(llabs(args[0].i));
        return val_real(fabs(args[0].d));
    }
    if (strcmp(name, "floor") == 0) {
        if (argc != 1) bma_error("floor requires 1 argument");
        double d = (args[0].type == V_INT) ? (double)args[0].i : args[0].d;
        return val_int((long long)floor(d));
    }
    if (strcmp(name, "ceil") == 0) {
        if (argc != 1) bma_error("ceil requires 1 argument");
        double d = (args[0].type == V_INT) ? (double)args[0].i : args[0].d;
        return val_int((long long)ceil(d));
    }
    if (strcmp(name, "round") == 0) {
        if (argc != 1) bma_error("round requires 1 argument");
        double d = (args[0].type == V_INT) ? (double)args[0].i : args[0].d;
        return val_int((long long)round(d));
    }
    if (strcmp(name, "pow") == 0) {
        if (argc != 2) bma_error("pow requires 2 arguments");
        double a = (args[0].type == V_INT) ? (double)args[0].i : args[0].d;
        double b = (args[1].type == V_INT) ? (double)args[1].i : args[1].d;
        return val_real(pow(a, b));
    }
    if (strcmp(name, "sin") == 0) {
        if (argc != 1) bma_error("sin requires 1 argument");
        double d = (args[0].type == V_INT) ? (double)args[0].i : args[0].d;
        return val_real(sin(d));
    }
    if (strcmp(name, "cos") == 0) {
        if (argc != 1) bma_error("cos requires 1 argument");
        double d = (args[0].type == V_INT) ? (double)args[0].i : args[0].d;
        return val_real(cos(d));
    }
    if (strcmp(name, "tan") == 0) {
        if (argc != 1) bma_error("tan requires 1 argument");
        double d = (args[0].type == V_INT) ? (double)args[0].i : args[0].d;
        return val_real(tan(d));
    }
    if (strcmp(name, "atan2") == 0) {
        if (argc != 2) bma_error("atan2 requires 2 arguments");
        double y = (args[0].type == V_INT) ? (double)args[0].i : args[0].d;
        double x = (args[1].type == V_INT) ? (double)args[1].i : args[1].d;
        return val_real(atan2(y, x));
    }
    if (strcmp(name, "log") == 0) {
        if (argc != 1) bma_error("log requires 1 argument");
        double d = (args[0].type == V_INT) ? (double)args[0].i : args[0].d;
        return val_real(log(d));
    }
    if (strcmp(name, "log10") == 0) {
        if (argc != 1) bma_error("log10 requires 1 argument");
        double d = (args[0].type == V_INT) ? (double)args[0].i : args[0].d;
        return val_real(log10(d));
    }
    if (strcmp(name, "exp") == 0) {
        if (argc != 1) bma_error("exp requires 1 argument");
        double d = (args[0].type == V_INT) ? (double)args[0].i : args[0].d;
        return val_real(exp(d));
    }
    if (strcmp(name, "min") == 0) {
        if (argc != 2) bma_error("min requires 2 arguments");
        if (args[0].type == V_INT && args[1].type == V_INT)
            return val_int(args[0].i < args[1].i ? args[0].i : args[1].i);
        double a = (args[0].type == V_INT) ? (double)args[0].i : args[0].d;
        double b = (args[1].type == V_INT) ? (double)args[1].i : args[1].d;
        return val_real(a < b ? a : b);
    }
    if (strcmp(name, "max") == 0) {
        if (argc != 2) bma_error("max requires 2 arguments");
        if (args[0].type == V_INT && args[1].type == V_INT)
            return val_int(args[0].i > args[1].i ? args[0].i : args[1].i);
        double a = (args[0].type == V_INT) ? (double)args[0].i : args[0].d;
        double b = (args[1].type == V_INT) ? (double)args[1].i : args[1].d;
        return val_real(a > b ? a : b);
    }
    if (strcmp(name, "random") == 0) {
        return val_real((double)rand() / (double)RAND_MAX);
    }
    if (strcmp(name, "random_int") == 0) {
        if (argc != 1 || args[0].type != V_INT) bma_error("random_int requires 1 integer");
        if (args[0].i <= 0) bma_error("random_int requires n > 0");
        return val_int(rand() % args[0].i);
    }

    // ═══════════════════════════════════════════════════════════
    // STRINGS
    // ═══════════════════════════════════════════════════════════
    if (strcmp(name, "length") == 0) {
        if (argc != 1 || args[0].type != V_STRING) bma_error("length requires a string");
        return val_int(strlen(args[0].s));
    }
    if (strcmp(name, "upper") == 0) {
        if (argc != 1 || args[0].type != V_STRING) bma_error("upper requires a string");
        char *buf = my_strdup(args[0].s);
        for (int i = 0; buf[i]; i++) buf[i] = toupper(buf[i]);
        return val_str(buf);
    }
    if (strcmp(name, "lower") == 0) {
        if (argc != 1 || args[0].type != V_STRING) bma_error("lower requires a string");
        char *buf = my_strdup(args[0].s);
        for (int i = 0; buf[i]; i++) buf[i] = tolower(buf[i]);
        return val_str(buf);
    }
    if (strcmp(name, "trim") == 0) {
        if (argc != 1 || args[0].type != V_STRING) bma_error("trim requires a string");
        const char *s = args[0].s;
        while (*s && isspace((unsigned char)*s)) s++;
        int len = (int)strlen(s);
        while (len > 0 && isspace((unsigned char)s[len - 1])) len--;
        char *buf = malloc(len + 1);
        memcpy(buf, s, len);
        buf[len] = 0;
        return val_str(buf);
    }
    if (strcmp(name, "substring") == 0) {
        if (argc != 3 || args[0].type != V_STRING || args[1].type != V_INT || args[2].type != V_INT)
            bma_error("substring requires (string, start, len)");
        int start = (int)args[1].i;
        int len = (int)args[2].i;
        int slen = strlen(args[0].s);
        if (start < 0) start = 0;
        if (start >= slen) return val_str(my_strdup(""));
        if (start + len > slen) len = slen - start;
        char *buf = malloc(len + 1);
        memcpy(buf, args[0].s + start, len);
        buf[len] = 0;
        return val_str(buf);
    }
    if (strcmp(name, "contains") == 0) {
        if (argc != 2 || args[0].type != V_STRING || args[1].type != V_STRING)
            bma_error("contains requires 2 strings");
        return val_bool(strstr(args[0].s, args[1].s) != NULL);
    }
    if (strcmp(name, "index_of") == 0) {
        if (argc != 2 || args[0].type != V_STRING || args[1].type != V_STRING)
            bma_error("index_of requires 2 strings");
        char *pos = strstr(args[0].s, args[1].s);
        return val_int(pos ? (pos - args[0].s) : -1);
    }
    if (strcmp(name, "starts_with") == 0) {
        if (argc != 2 || args[0].type != V_STRING || args[1].type != V_STRING)
            bma_error("starts_with requires 2 strings");
        return val_bool(strncmp(args[0].s, args[1].s, strlen(args[1].s)) == 0);
    }
    if (strcmp(name, "ends_with") == 0) {
        if (argc != 2 || args[0].type != V_STRING || args[1].type != V_STRING)
            bma_error("ends_with requires 2 strings");
        size_t slen = strlen(args[0].s), plen = strlen(args[1].s);
        if (plen > slen) return val_bool(false);
        return val_bool(strcmp(args[0].s + slen - plen, args[1].s) == 0);
    }
    if (strcmp(name, "replace") == 0) {
        if (argc != 3 || args[0].type != V_STRING || args[1].type != V_STRING || args[2].type != V_STRING)
            bma_error("replace requires 3 strings");
        const char *s = args[0].s, *old = args[1].s, *new = args[2].s;
        size_t old_len = strlen(old), new_len = strlen(new);
        if (old_len == 0) return val_str(my_strdup(s));
        size_t cap = strlen(s) + 1, len = 0;
        char *buf = malloc(cap);
        const char *p = s;
        while (*p) {
            if (strncmp(p, old, old_len) == 0) {
                if (len + new_len + 1 > cap) { cap = (len + new_len + 1) * 2; buf = realloc(buf, cap); }
                memcpy(buf + len, new, new_len); len += new_len; p += old_len;
            } else {
                if (len + 2 > cap) { cap *= 2; buf = realloc(buf, cap); }
                buf[len++] = *p++;
            }
        }
        buf[len] = 0;
        return val_str(buf);
    }
    if (strcmp(name, "char_at") == 0) {
        if (argc != 2 || args[0].type != V_STRING || args[1].type != V_INT)
            bma_error("char_at requires (string, integer)");
        int idx = (int)args[1].i;
        if (idx < 0 || idx >= (int)strlen(args[0].s)) return val_str(my_strdup(""));
        char buf[2] = { args[0].s[idx], 0 };
        return val_str(my_strdup(buf));
    }
    if (strcmp(name, "repeat_str") == 0) {
        if (argc != 2 || args[0].type != V_STRING || args[1].type != V_INT)
            bma_error("repeat_str requires (string, integer)");
        int n = (int)args[1].i;
        size_t slen = strlen(args[0].s);
        char *buf = malloc(slen * n + 1);
        for (int i = 0; i < n; i++) memcpy(buf + i * slen, args[0].s, slen);
        buf[slen * n] = 0;
        return val_str(buf);
    }
    if (strcmp(name, "reverse") == 0) {
        if (argc != 1 || args[0].type != V_STRING) bma_error("reverse requires a string");
        size_t len = strlen(args[0].s);
        char *buf = malloc(len + 1);
        for (size_t i = 0; i < len; i++) buf[i] = args[0].s[len - 1 - i];
        buf[len] = 0;
        return val_str(buf);
    }

    // ═══════════════════════════════════════════════════════════
    // TEXT HELPERS
    // ═══════════════════════════════════════════════════════════
    if (strcmp(name, "newline") == 0) return val_str(my_strdup("\n"));
    if (strcmp(name, "tab") == 0) return val_str(my_strdup("\t"));

    // ═══════════════════════════════════════════════════════════
    // FILES
    // ═══════════════════════════════════════════════════════════
    if (strcmp(name, "read_file") == 0) {
        if (argc != 1 || args[0].type != V_STRING) bma_error("read_file requires a path");
        FILE *f = fopen(args[0].s, "rb");
        if (!f) bma_error("Cannot open file '%s'", args[0].s);
        fseek(f, 0, SEEK_END);
        long sz = ftell(f);
        fseek(f, 0, SEEK_SET);
        char *buf = malloc(sz + 1);
        fread(buf, 1, sz, f);
        buf[sz] = 0;
        fclose(f);
        return val_str(buf);
    }
    if (strcmp(name, "write_file") == 0) {
        if (argc != 2 || args[0].type != V_STRING || args[1].type != V_STRING)
            bma_error("write_file requires (path, content)");
        FILE *f = fopen(args[0].s, "wb");
        if (!f) bma_error("Cannot create '%s'", args[0].s);
        fwrite(args[1].s, 1, strlen(args[1].s), f);
        fclose(f);
        return val_null();
    }
    if (strcmp(name, "append_file") == 0) {
        if (argc != 2 || args[0].type != V_STRING || args[1].type != V_STRING)
            bma_error("append_file requires (path, content)");
        FILE *f = fopen(args[0].s, "ab");
        if (!f) bma_error("Cannot open '%s'", args[0].s);
        fwrite(args[1].s, 1, strlen(args[1].s), f);
        fclose(f);
        return val_null();
    }
    if (strcmp(name, "file_exists") == 0) {
        if (argc != 1 || args[0].type != V_STRING) bma_error("file_exists requires a path");
        FILE *f = fopen(args[0].s, "r");
        if (f) { fclose(f); return val_bool(true); }
        return val_bool(false);
    }
    if (strcmp(name, "delete_file") == 0) {
        if (argc != 1 || args[0].type != V_STRING) bma_error("delete_file requires a path");
        return val_bool(remove(args[0].s) == 0);
    }

    // ═══════════════════════════════════════════════════════════
    // HTTP
    // ═══════════════════════════════════════════════════════════
    if (strcmp(name, "http_get") == 0) {
        if (argc != 1 || args[0].type != V_STRING) bma_error("http_get requires a URL");
        CURL *curl = curl_easy_init();
        if (!curl) return val_str(my_strdup(""));
        CurlBuf buf = { malloc(1), 0 };
        buf.data[0] = 0;
        curl_easy_setopt(curl, CURLOPT_URL, args[0].s);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "BMA-C/1.0");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "Warning: HTTP request failed: %s\n", curl_easy_strerror(res));
            free(buf.data);
            return val_str(my_strdup(""));
        }
        return val_str(buf.data);
    }
    if (strcmp(name, "url_encode") == 0) {
        if (argc != 1 || args[0].type != V_STRING) bma_error("url_encode requires a string");
        CURL *c = curl_easy_init();
        char *enc = curl_easy_escape(c, args[0].s, 0);
        char *buf = my_strdup(enc ? enc : "");
        if (enc) curl_free(enc);
        curl_easy_cleanup(c);
        return val_str(buf);
    }
    if (strcmp(name, "url_decode") == 0) {
        if (argc != 1 || args[0].type != V_STRING) bma_error("url_decode requires a string");
        CURL *c = curl_easy_init();
        int outlen = 0;
        char *dec = curl_easy_unescape(c, args[0].s, 0, &outlen);
        char *buf = malloc(outlen + 1);
        if (dec) { memcpy(buf, dec, outlen); curl_free(dec); }
        buf[outlen] = 0;
        curl_easy_cleanup(c);
        return val_str(buf);
    }

    // ═══════════════════════════════════════════════════════════
    // JSON
    // ═══════════════════════════════════════════════════════════
    if (strcmp(name, "json_valid") == 0) {
        if (argc != 1 || args[0].type != V_STRING) bma_error("json_valid requires a string");
        cJSON *root = cJSON_Parse(args[0].s);
        if (root) { cJSON_Delete(root); return val_bool(true); }
        return val_bool(false);
    }
    if (strcmp(name, "json_has") == 0) {
        if (argc != 2 || args[0].type != V_STRING || args[1].type != V_STRING)
            bma_error("json_has requires (json, path)");
        cJSON *root = cJSON_Parse(args[0].s);
        if (!root) return val_bool(false);
        cJSON *item = json_find(root, args[1].s);
        bool has = (item != NULL);
        cJSON_Delete(root);
        return val_bool(has);
    }
    if (strcmp(name, "json_get") == 0) {
        if (argc != 2 || args[0].type != V_STRING || args[1].type != V_STRING)
            bma_error("json_get requires (json, path)");
        cJSON *root = cJSON_Parse(args[0].s);
        if (!root) bma_error("Invalid JSON");
        cJSON *item = json_find(root, args[1].s);
        Value res = val_str(my_strdup(""));
        if (cJSON_IsString(item)) {
            res = val_str(my_strdup(item->valuestring));
        } else if (cJSON_IsNumber(item)) {
            char nb[64]; snprintf(nb, 64, "%g", item->valuedouble);
            res = val_str(my_strdup(nb));
        } else if (cJSON_IsBool(item)) {
            res = val_str(my_strdup(cJSON_IsTrue(item) ? "true" : "false"));
        } else if (cJSON_IsNull(item)) {
            res = val_str(my_strdup("null"));
        }
        cJSON_Delete(root);
        return res;
    }
    if (strcmp(name, "json_get_int") == 0) {
        if (argc != 2 || args[0].type != V_STRING || args[1].type != V_STRING)
            bma_error("json_get_int requires (json, path)");
        cJSON *root = cJSON_Parse(args[0].s);
        if (!root) bma_error("Invalid JSON");
        cJSON *item = json_find(root, args[1].s);
        int res = 0;
        if (cJSON_IsNumber(item)) res = item->valueint;
        else if (cJSON_IsString(item)) res = atoi(item->valuestring);
        cJSON_Delete(root);
        return val_int(res);
    }
    if (strcmp(name, "json_get_real") == 0) {
        if (argc != 2 || args[0].type != V_STRING || args[1].type != V_STRING)
            bma_error("json_get_real requires (json, path)");
        cJSON *root = cJSON_Parse(args[0].s);
        if (!root) bma_error("Invalid JSON");
        cJSON *item = json_find(root, args[1].s);
        double res = 0.0;
        if (cJSON_IsNumber(item)) res = item->valuedouble;
        else if (cJSON_IsString(item)) res = atof(item->valuestring);
        cJSON_Delete(root);
        return val_real(res);
    }
    if (strcmp(name, "json_get_bool") == 0) {
        if (argc != 2 || args[0].type != V_STRING || args[1].type != V_STRING)
            bma_error("json_get_bool requires (json, path)");
        cJSON *root = cJSON_Parse(args[0].s);
        if (!root) bma_error("Invalid JSON");
        cJSON *item = json_find(root, args[1].s);
        bool res = cJSON_IsTrue(item);
        cJSON_Delete(root);
        return val_bool(res);
    }

    *found = false;
    return val_null();
}