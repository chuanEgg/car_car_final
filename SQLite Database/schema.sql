DROP TABLE IF EXISTS Location_Names;
DROP TABLE IF EXISTS Controls;
/*
DROP TABLE IF EXISTS City_Weather;
DROP TABLE IF EXISTS Location_Weather;
*/

CREATE TABLE Location_Names (
    code INTEGER,
    Chinese_Name TEXT,
    English_Name TEXT,
    Weather_Forecast_Code_2_Days TEXT,
    Weather_Forecast_Code_7_Days TEXT
);

CREATE TABLE Controls (
    ctrl INTEGER,
    location_ctrl INTEGER
);

/*
CREATE TABLE City_Weather (
    code INTEGER,
    Chinese_Name TEXT,
    English_Name TEXT,
    Wx_12hr_1 INTEGER,
    Wx_12hr_2 INTEGER,
    Wx_12hr_3 INTEGER,
    PoP_12hr_1 INTEGER,
    PoP_12hr_2 INTEGER,
    PoP_12hr_3 INTEGER,
    MinT_12hr_1 INTEGER,
    MinT_12hr_2 INTEGER,
    MinT_12hr_3 INTEGER,
    MaxT_12hr_1 INTEGER,
    MaxT_12hr_2 INTEGER,
    MaxT_12hr_3 INTEGER,
    CI_12hr_1 TEXT,
    CI_12hr_2 TEXT,
    CI_12hr_3 TEXT,
);

CREATE TABLE Location_Weather (
    code INTEGER,
    Chinese_Name TEXT,
    English_Name TEXT,
    Wx_12hr_1 INTEGER,
    Wx_12hr_2 INTEGER,
    Wx_12hr_3 INTEGER,
    PoP_12hr_1 INTEGER,
    PoP_12hr_2 INTEGER,
    PoP_12hr_3 INTEGER,
    MinT_12hr_1 INTEGER,
    MinT_12hr_2 INTEGER,
    MinT_12hr_3 INTEGER,
    MaxT_12hr_1 INTEGER,
    MaxT_12hr_2 INTEGER,
    MaxT_12hr_3 INTEGER,
    CI_12hr_1 TEXT,
    CI_12hr_2 TEXT,
    CI_12hr_3 TEXT,
);
*/
