-- TPC-H Query 17: Small-Quantity Order Revenue
-- Average yearly revenue from Brand#23 MED BOX parts ordered below 20% of average quantity.
SELECT SUM(l_extendedprice) / 7.0 AS avg_yearly
FROM lineitem, part
WHERE p_partkey = l_partkey
  AND p_brand = 'Brand#23'
  AND p_container = 'MED BOX'
  AND l_quantity < (
    SELECT 0.2 * AVG(l_quantity)
    FROM lineitem
    WHERE l_partkey = p_partkey
  )
