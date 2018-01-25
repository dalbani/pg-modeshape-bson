-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION modeshape_bson" to load this file. \quit

CREATE FUNCTION modeshape_bson_version()
    RETURNS text
    AS 'MODULE_PATHNAME'
    LANGUAGE C STRICT IMMUTABLE;


CREATE FUNCTION json_text_to_modeshape_bson(text)
    RETURNS bytea
    AS 'MODULE_PATHNAME'
    LANGUAGE C STRICT IMMUTABLE;

CREATE FUNCTION modeshape_bson_to_json_text(bytea)
    RETURNS text
    AS 'MODULE_PATHNAME'
    LANGUAGE C STRICT IMMUTABLE;

