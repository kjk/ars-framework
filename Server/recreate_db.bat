c:\mysql\bin\mysql.exe -u root <recreate_enwiki.sql
c:\mysql\bin\mysql.exe -u root enwiki <c:\20040313_cur_table.sql
c:\mysql\bin\mysql.exe -u root <recreate_ipedia.sql
python converter.py
