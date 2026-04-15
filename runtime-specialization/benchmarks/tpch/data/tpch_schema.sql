-- TPC-H schema for SQLite
-- All numeric types use REAL/INTEGER as SQLite has flexible typing.

CREATE TABLE IF NOT EXISTS region (
    r_regionkey  INTEGER PRIMARY KEY,
    r_name       TEXT    NOT NULL,
    r_comment    TEXT
);

CREATE TABLE IF NOT EXISTS nation (
    n_nationkey  INTEGER PRIMARY KEY,
    n_name       TEXT    NOT NULL,
    n_regionkey  INTEGER NOT NULL REFERENCES region(r_regionkey),
    n_comment    TEXT
);

CREATE TABLE IF NOT EXISTS part (
    p_partkey     INTEGER PRIMARY KEY,
    p_name        TEXT    NOT NULL,
    p_mfgr        TEXT    NOT NULL,
    p_brand       TEXT    NOT NULL,
    p_type        TEXT    NOT NULL,
    p_size        INTEGER NOT NULL,
    p_container   TEXT    NOT NULL,
    p_retailprice REAL    NOT NULL,
    p_comment     TEXT    NOT NULL
);

CREATE TABLE IF NOT EXISTS supplier (
    s_suppkey   INTEGER PRIMARY KEY,
    s_name      TEXT    NOT NULL,
    s_address   TEXT    NOT NULL,
    s_nationkey INTEGER NOT NULL REFERENCES nation(n_nationkey),
    s_phone     TEXT    NOT NULL,
    s_acctbal   REAL    NOT NULL,
    s_comment   TEXT    NOT NULL
);

CREATE TABLE IF NOT EXISTS partsupp (
    ps_partkey    INTEGER NOT NULL REFERENCES part(p_partkey),
    ps_suppkey    INTEGER NOT NULL REFERENCES supplier(s_suppkey),
    ps_availqty   INTEGER NOT NULL,
    ps_supplycost REAL    NOT NULL,
    ps_comment    TEXT    NOT NULL,
    PRIMARY KEY (ps_partkey, ps_suppkey)
);

CREATE TABLE IF NOT EXISTS customer (
    c_custkey    INTEGER PRIMARY KEY,
    c_name       TEXT    NOT NULL,
    c_address    TEXT    NOT NULL,
    c_nationkey  INTEGER NOT NULL REFERENCES nation(n_nationkey),
    c_phone      TEXT    NOT NULL,
    c_acctbal    REAL    NOT NULL,
    c_mktsegment TEXT    NOT NULL,
    c_comment    TEXT    NOT NULL
);

CREATE TABLE IF NOT EXISTS orders (
    o_orderkey      INTEGER PRIMARY KEY,
    o_custkey       INTEGER NOT NULL REFERENCES customer(c_custkey),
    o_orderstatus   TEXT    NOT NULL,
    o_totalprice    REAL    NOT NULL,
    o_orderdate     TEXT    NOT NULL,
    o_orderpriority TEXT    NOT NULL,
    o_clerk         TEXT    NOT NULL,
    o_shippriority  INTEGER NOT NULL,
    o_comment       TEXT    NOT NULL
);

CREATE TABLE IF NOT EXISTS lineitem (
    l_orderkey      INTEGER NOT NULL REFERENCES orders(o_orderkey),
    l_partkey       INTEGER NOT NULL,
    l_suppkey       INTEGER NOT NULL,
    l_linenumber    INTEGER NOT NULL,
    l_quantity      REAL    NOT NULL,
    l_extendedprice REAL    NOT NULL,
    l_discount      REAL    NOT NULL,
    l_tax           REAL    NOT NULL,
    l_returnflag    TEXT    NOT NULL,
    l_linestatus    TEXT    NOT NULL,
    l_shipdate      TEXT    NOT NULL,
    l_commitdate    TEXT    NOT NULL,
    l_receiptdate   TEXT    NOT NULL,
    l_shipinstruct  TEXT    NOT NULL,
    l_shipmode      TEXT    NOT NULL,
    l_comment       TEXT    NOT NULL,
    PRIMARY KEY (l_orderkey, l_linenumber)
);

-- Indexes for the benchmark queries
CREATE INDEX IF NOT EXISTS idx_lineitem_shipdate ON lineitem(l_shipdate);
CREATE INDEX IF NOT EXISTS idx_orders_orderdate  ON orders(o_orderdate);
CREATE INDEX IF NOT EXISTS idx_orders_custkey    ON orders(o_custkey);
CREATE INDEX IF NOT EXISTS idx_customer_mkt      ON customer(c_mktsegment);
