-- TPC-H Query 4: Order Priority Checking
-- Counts orders by priority for orders placed in Q3 1993 where at least one
-- line item was received late.
SELECT o_orderpriority, COUNT(*) AS order_count
FROM orders
WHERE o_orderdate >= '1993-07-01'
  AND o_orderdate < '1993-10-01'
  AND EXISTS (
    SELECT * FROM lineitem
    WHERE l_orderkey = o_orderkey
      AND l_commitdate < l_receiptdate
  )
GROUP BY o_orderpriority
ORDER BY o_orderpriority
