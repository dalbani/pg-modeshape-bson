MONGO_C_DRIVER_DIR = mongo-c-driver
LIBBSON_DIR = $(MONGO_C_DRIVER_DIR)/src/libbson
LIBBSON_STATIC_LIB = $(LIBBSON_DIR)/.libs/libbson.a

EXTENSION = modeshape_bson
MODULE_big = modeshape_bson
DATA = modeshape_bson--1.0.sql
OBJS = modeshape_bson.o $(LIBBSON_STATIC_LIB)
PG_CPPFLAGS = -I$(LIBBSON_DIR)/src/bson/

all: $(LIBBSON_STATIC_LIB) modeshape_bson.o

.mongo-c-driver.patched:
	patch --directory $(MONGO_C_DRIVER_DIR) -p1 < mongo-c-driver.patch && touch $@

$(LIBBSON_STATIC_LIB): .mongo-c-driver.patched
	cd $(LIBBSON_DIR) && export AUTOGEN_CONFIGURE_ARGS="--disable-tests --disable-examples" && ./autogen.sh && make

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

