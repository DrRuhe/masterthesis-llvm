SELECT o_custkey, SUM(l_extendedprice) FROM orders JOIN lineitem ON o_orderkey = l_orderkey GROUP BY o_custkey
