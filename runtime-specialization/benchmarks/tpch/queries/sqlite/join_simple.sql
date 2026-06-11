SELECT o_orderkey, l_quantity FROM orders JOIN lineitem ON o_orderkey = l_orderkey LIMIT 1000
