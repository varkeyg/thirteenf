select distinct h.holder_cik as id, 'holder' as label, holder_name from holdings h;

select distinct 'period' as label, period_date from holdings;

select cusip, period_date ,holding_name , sum(market_value) as market_value, sum(quantity) as quantity, qty_type  from holdings h
group by cusip, period_date , holding_name , qty_type 