c:\mysql\bin\mysql.exe -u root <recreate_ipedia.sql
c:\mysql\bin\mysql.exe -u root ipedia <iPediaDatabase-no-ft.sql
python converter.py -usepsyco -fromsql G:\wikipedia\20040417_cur_table.sql
c:\mysql\bin\mysql.exe -u root ipedia <create-ft.sql
