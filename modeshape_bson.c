#include <postgres.h>
#include <utils/builtins.h>
#include <bson.h>

PG_MODULE_MAGIC;


PG_FUNCTION_INFO_V1(modeshape_bson_version);
Datum modeshape_bson_version(PG_FUNCTION_ARGS)
{
    PG_RETURN_TEXT_P(cstring_to_text("1.0 libbson/" BSON_VERSION_S));
}


// ModeShape BSON (bytea) to JSON (text)
PG_FUNCTION_INFO_V1(modeshape_bson_to_json_text);
Datum
modeshape_bson_to_json_text(PG_FUNCTION_ARGS)
{
    bytea* bson_arg;
    uint8_t* bson_data;
    Size bson_data_size;
    bson_reader_t* bson_reader;
    bool bson_reader_reached_eof;
    const bson_t* bson_doc;
    char* json_data;
    size_t json_data_size;
    text* json_text_ret;

    if (PG_ARGISNULL(0)) {
        PG_RETURN_NULL();
    }

    bson_arg = PG_GETARG_BYTEA_PP(0);
    bson_data = (uint8_t*) VARDATA(bson_arg);
    bson_data_size = VARSIZE(bson_arg) - VARHDRSZ;

    bson_reader = bson_reader_new_from_data(bson_data, bson_data_size);
    bson_doc = bson_reader_read(bson_reader, &bson_reader_reached_eof);

    if (!bson_doc || bson_reader_reached_eof) {
        bson_reader_destroy(bson_reader);
        elog(ERROR, "Failed to convert BSON to JSON.");
    }

    json_data = bson_as_canonical_extended_json(bson_doc, &json_data_size);

    bson_reader_destroy(bson_reader);

    json_text_ret = cstring_to_text_with_len(json_data, json_data_size);

    bson_free(json_data);

    PG_RETURN_TEXT_P(json_text_ret);
}


// JSON (text) to ModeShape BSON (bytea)
PG_FUNCTION_INFO_V1(json_text_to_modeshape_bson);
Datum
json_text_to_modeshape_bson(PG_FUNCTION_ARGS)
{
    text *json_text_arg;
    char* json_data;
    size_t json_data_size;
    bson_t bson_doc = BSON_INITIALIZER;
    Size bson_data_size;
    const uint8_t* bson_data;
    bytea* bson_ret;

    if (PG_ARGISNULL(0)) {
        PG_RETURN_NULL();
    }

    json_text_arg = PG_GETARG_TEXT_PP(0);
    json_data = VARDATA(json_text_arg);
    json_data_size = VARSIZE(json_text_arg) - VARHDRSZ;

    if (!bson_init_from_json(&bson_doc, json_data, json_data_size, NULL)) {
        bson_destroy(&bson_doc);
        elog(ERROR, "Failed to convert JSON to BSON.");
    }

    bson_data = bson_get_data(&bson_doc);
    bson_data_size = bson_doc.len;

    bson_ret = (bytea *) palloc(bson_data_size + VARHDRSZ);
    SET_VARSIZE(bson_ret, bson_data_size + VARHDRSZ);
    memcpy(VARDATA(bson_ret), bson_data, bson_data_size);

    bson_destroy(&bson_doc);

    PG_RETURN_BYTEA_P(bson_ret);
}

