SET AUTOCOMMIT=0;
USE alienvault;


REPLACE INTO config (conf, value) VALUES ('latest_asset_change', utc_timestamp());
REPLACE INTO config (conf, value) VALUES ('last_update', '2020-12-16');
REPLACE INTO config (conf, value) VALUES ('ossim_schema_version', '5.8.6');

-- PLEASE ADD NOTHING HERE

COMMIT;
-- NOTHING BELOW THIS LINE / NADA DEBAJO DE ESTA LINEA
