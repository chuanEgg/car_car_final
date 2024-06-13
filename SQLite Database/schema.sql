DROP TABLE IF EXISTS Location_Names; --鄉鎮市區地區
DROP TABLE IF EXISTS Controls; --換頁、控制

DROP TABLE IF EXISTS City_Weather_36hr; --僅縣市地區天氣36hr預報
DROP TABLE IF EXISTS City_Township_Weather_3_Days; --各鄉鎮市區逐12小時天氣，共3天，由7 days擷取

CREATE TABLE Location_Names (
    id INTEGER PRIMARY KEY ,
    Postal_Code TEXT ,
    Chinese_Name TEXT,
    English_Name TEXT,
    Weather_Forecast_Code_36_Hours_ID TEXT,
    Weather_Forecast_Code_2_Days_ID TEXT,
    Weather_Forecast_Code_7_Days_ID TEXT
);

CREATE TABLE Controls (
    page_ctrl INTEGER DEFAULT 0, -- control how much page to advance or go back
    page_activation INTEGER DEFAULT -2147483648, -- control which page to show, use bit number n to indicate whether page n is on
    location_id_ctrl INTEGER DEFAULT 0, --store current location code
    motor_pwm_ctrl INTEGER DEFAULT 1, --store motor max pwm
    motor_time_duration_ctrl INTEGER DEFAULT 30, --store motor time duration
    fan_pwm_ctrl INTEGER DEFAULT 1, --store fan max pwm
    fan_time_duration_ctrl INTEGER DEFAULT 30 --store fan time duration

);


CREATE TABLE City_Weather_36hr (
    id INTEGER PRIMARY KEY ,
    Postal_Code TEXT,
    Chinese_Name TEXT,
    English_Name TEXT,
    
    Wx_12hr_0 INTEGER DEFAULT 0,
    Wx_12hr_1 INTEGER DEFAULT 0,
    Wx_12hr_2 INTEGER DEFAULT 0,

    PoP_12hr_0 INTEGER DEFAULT 0,
    PoP_12hr_1 INTEGER DEFAULT 0,
    PoP_12hr_2 INTEGER DEFAULT 0,

    MinT_12hr_0 INTEGER DEFAULT 0,
    MinT_12hr_1 INTEGER DEFAULT 0,
    MinT_12hr_2 INTEGER DEFAULT 0,

    MaxT_12hr_0 INTEGER DEFAULT 0,
    MaxT_12hr_1 INTEGER DEFAULT 0,
    MaxT_12hr_2 INTEGER DEFAULT 0,

    startTime_0 TEXT,
    startTime_1 TEXT,
    startTime_2 TEXT,

    endTime_0 TEXT,
    endTime_1 TEXT,
    endTime_2 TEXT,

    Time_Of_Last_Update_In_Seconds BIGINT DEFAULT 0
);

CREATE TABLE City_Township_Weather_3_Days (
    id INTEGER PRIMARY KEY ,
    Postal_Code TEXT,
    Chinese_Name TEXT,
    English_Name TEXT,

    Wx_12hr_0 INTEGER DEFAULT 0,
    Wx_12hr_1 INTEGER DEFAULT 0,
    Wx_12hr_2 INTEGER DEFAULT 0,
    Wx_12hr_3 INTEGER DEFAULT 0,
    Wx_12hr_4 INTEGER DEFAULT 0,
    Wx_12hr_5 INTEGER DEFAULT 0,
    
    PoP_12hr_0 INTEGER DEFAULT 0,
    PoP_12hr_1 INTEGER DEFAULT 0,
    PoP_12hr_2 INTEGER DEFAULT 0,
    PoP_12hr_3 INTEGER DEFAULT 0,
    PoP_12hr_4 INTEGER DEFAULT 0,
    PoP_12hr_5 INTEGER DEFAULT 0,

    MinT_12hr_0 INTEGER DEFAULT 0,
    MinT_12hr_1 INTEGER DEFAULT 0,
    MinT_12hr_2 INTEGER DEFAULT 0,
    MinT_12hr_3 INTEGER DEFAULT 0,
    MinT_12hr_4 INTEGER DEFAULT 0,
    MinT_12hr_5 INTEGER DEFAULT 0,

    MaxT_12hr_0 INTEGER DEFAULT 0,
    MaxT_12hr_1 INTEGER DEFAULT 0,
    MaxT_12hr_2 INTEGER DEFAULT 0,
    MaxT_12hr_3 INTEGER DEFAULT 0,
    MaxT_12hr_4 INTEGER DEFAULT 0,
    MaxT_12hr_5 INTEGER DEFAULT 0,

    startTime_0 TEXT,
    startTime_1 TEXT,
    startTime_2 TEXT,
    startTime_3 TEXT,
    startTime_4 TEXT,
    startTime_5 TEXT,

    endTime_0 TEXT,
    endTime_1 TEXT,
    endTime_2 TEXT,
    endTime_3 TEXT,
    endTime_4 TEXT,
    endTime_5 TEXT,

    Time_Of_Last_Update_In_Seconds BIGINT DEFAULT 0
);
