c:\mysql\bin\mysql.exe -u root <recreate_enwiki.sql
@rem c:\mysql\bin\mysql.exe -u root enwiki <c:\20040313_cur_table.sql
c:\mysql\bin\mysql.exe -u root enwiki <c:\20040129_cur_table.sql
c:\mysql\bin\mysql.exe -u root <recreate_ipedia.sql
c:\mysql\bin\mysql.exe -u root ipedia <iPediaDatabase-no-ft.sql
python converter.py
c:\mysql\bin\mysql.exe -u root ipedia <create-ft.sql
