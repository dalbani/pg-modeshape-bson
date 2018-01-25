# pg-modeshape-bson
PostgreSQL extension to convert ModeShape BSON to/from JSON.

## Purpose

To be able to transparently decode/encode binary BSON data from ModeShape, in order to store it as JSON in PostgreSQL.

## How to use

1. Install the extension in PostgreSQL:

```
$ git clone --recursive https://github.com/dalbani/pg-modeshape-bson.git
$ cd pg-modeshape-bson
$ make
$ sudo make install
```

2. Create a proxy table in PostgreSQL (a.k.a "writable view"):

```sql
CREATE TABLE modeshape
(
  id text NOT NULL,
  last_changed timestamp without time zone NOT NULL DEFAULT now(),
  content jsonb NOT NULL,
  CONSTRAINT modeshape_pkey PRIMARY KEY (id)
);


CREATE TABLE modeshape_proxy
(
  id text NOT NULL,
  last_changed timestamp without time zone NOT NULL,
  content bytea NOT NULL
);


CREATE OR REPLACE RULE "_RETURN" AS
  ON SELECT TO modeshape_proxy
  DO INSTEAD
  SELECT id,
         last_changed,
         json_text_to_modeshape_bson(content::text) AS content
    FROM modeshape;


CREATE OR REPLACE RULE insert_modeshape_proxy_rule AS
  ON INSERT TO modeshape_proxy
  DO INSTEAD
     INSERT INTO modeshape(id, last_changed, content)
     VALUES (NEW.id,
             NEW.last_changed,
             modeshape_bson_to_json_text(NEW.content)::jsonb);


CREATE OR REPLACE RULE update_modeshape_proxy_rule AS
  ON UPDATE TO modeshape_proxy
  DO INSTEAD
     UPDATE modeshape
        SET last_changed = NEW.last_changed,
            content = modeshape_bson_to_json_text(NEW.content)::jsonb
      WHERE id = OLD.id;


CREATE OR REPLACE RULE delete_modeshape_proxy_rule AS
  ON DELETE TO modeshape_proxy
  DO INSTEAD
     DELETE FROM modeshape
      WHERE id = OLD.id;
```

3. Configure ModeShape to use the proxy table:

```xml
<db-persistence table-name="modeshape_proxy" url="jdbc:postgresql://localhost:5432/modeshape" driver="org.postgresql.Driver" username="modeshape" password="modeshape" create-on-start="false" compress="false" />
```

## Known issues

While `$date` properties are almost exclusively stored as an ISO 8601 string in BSON documents produced by ModeShape,
it appears that the database record with ID `"repository:info"` contains a `$date` property using the binary representation
(type 9).

It doesn't seem to cause any problem though (as long as this database record is not written??).
