c:\mysql\bin\mysql.exe -u root <recreate_ipedia.sql
c:\mysql\bin\mysql.exe -u root ipedia <iPediaDatabase-no-ft.sql
python newConvert.py G:\wikipedia\20040424_cur_table.sql.bz2
c:\mysql\bin\mysql.exe -u root ipedia <create-ft.sql
