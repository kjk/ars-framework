c:\mysql\bin\mysql.exe -u root <recreate_ipedia.sql
c:\mysql\bin\mysql.exe -u root ipedia <iPediaDatabase-no-ft.sql
python converter.py
c:\mysql\bin\mysql.exe -u root ipedia <create-ft.sql