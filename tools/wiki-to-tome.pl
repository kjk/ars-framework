#!/usr/bin/perl
############################################
# version 1.3
# - Unicode support added
#   First unicode characters are translated to &#nnn; type html codes
#   Then in two cases html codes are converted to ascii codes (0x80-0xFF) or
#   ascii substitutes (often unaccented version of character):
#   1 Palm: all text
#     Palm does not support unicode natively (some app's do, TomeRaider does not)
#   2 Pocket PC: only index entries (=titles) + links
#     PPC does support unicode, but TomeRaider shows html tags in index in raw format,
#     renders them only in body text, therefore index shows ascii substitutes, and by necessity links as well.
#   (see for very complete list of html codes also:
#     http://studweb.euv-frankfurt-o.de/twardoch/f/en/charsets/html4_0unicode2_0.html)
# - Esperanto (eo) support added
# - French (fr) support added
# - Bug fix: links to redirects containing underscores (~spaces) are finally recognized
# - dedicated EPOC version of English Wikipedia: one huge file like (P)PC version,
#   but without html tags, like Palm version
#
# version 1.2
# - Bug fix: links to redirects are now recognized as such
# - Bug fix: proper conversion date shown
# - Links to redirects are now also replaced by links to proper article
# - A link to the main page is added at the start of each article
# - Split file mode introduced for Palm OS:
#   Complete Wikipedia can be broken down into several volumes
#   Also html code is now removed for Palm users
#   Run with option palm, e.g. "perl WikiToTome.pl xx palm" (where xx = two letter language code)
# - (Pocket) PC version: text within tables is displayed in a fixed font size
#   This is a TomeRaider limitation. This font size has been scaled down now
#   for better viewing on a Pocket Pc.
#
# version 1.1
# = Performance:
#   html -> phase1 (ascii conversion) and phase3 (sort) optimised
#   + English Wikipedia will now use intermediate files to
#     minimize internal/virtual memory needed (my 512 Mb even proved insufficient)
#     -> execution now brought back from 82 to 46 minutes on my 500MHz PC
#    (extra file I/O amply compensated by less memory swapping)
#
# = Functionality:
#   - First article ('front page') extended: statistics and links to web pages added
#   - Main page edited: info stripped that is not so relevant on handheld version
#     = for example: sections 'Current events' and 'Writing Articles' are edited out
#       events are not so current any more in TomeRaider version
#       links about writing articles etc are not relevant here
#       and not available anyway (all meta info is stripped in this database)
#       result is a table of contents that is formatted well even on a small screen
#   - <nowiki>..</nowiki> tags parsed
#   - Links to images are now all removed
#   - Empty tables (after removing images) and
#     tables with just one text (99% image descriptions) are now removed

  $false = 0 ;
  $true  = 1 ;
  $version = "1.31" ;
  $testmode  = $false ;
  $testmode2 = $false ;

  use CGI::Carp qw(fatalsToBrowser);
  $| = 1; # flush screen output

  print "Convert Wikipedia sql dump to TomeRaider input file.\n" ;

  $language = lc (@ARGV [0]) ;
  if (($language ne "en") && ($language ne "de") && ($language ne "fr") &&
      ($language ne "eo") && ($language ne "nl"))
  {
    print "Please provide language code.\n" .
          "Currently supported: en, de, fr, eo, nl\n" ;
    exit ;
  }

  # Palm users: divide English Wikipedia in files < 30 Mb
  # Palm version of TomeRaider does not support html yet
  # (is not officially supported in PPC version either, but works there)
  $split_files = $false ;
  $remove_html = $false ;
  $remove_font_info = $false ;
  $mark_tables = $false ;
  $tables_small_font = $true ;

  $argv1 = lc (@ARGV [1]) ;
  $argv2 = lc (@ARGV [2]) ;
  $argv3 = lc (@ARGV [3]) ;

  if (($argv1 eq "palm")  || ($argv2 eq "palm") || ($argv3 eq "palm"))
  {
    $palm = $true ;
    $remove_html = $true ;
    $mark_tables = $true ;
    $tables_small_font = $false ;
  }
  else
  { $palm = $false ; }

  if (($argv1 eq "epoc") || ($argv2 eq "epoc") || ($argv3 eq "epoc"))
  {
    $epoc = $true ;
    $remove_html = $true ;
    $mark_tables = $true ;
    $tables_small_font = $false ;
  }
  else
  { $epoc = $false ; }

  if (($argv1 eq "demo") || ($argv2 eq "demo") || ($argv3 eq "demo"))
  { $demo = $true ; }
  else
  { $demo = $false ; }

  if (($argv1 eq "test") || ($argv2 eq "test") || ($argv3 eq "test"))
  { $testmode = $true ; }

  if (($argv1 eq "main") || ($argv2 eq "main") || ($argv3 eq "main"))
  {
    $testmain = $true ;
    $mainpage_found = $false ;
  }
  else
  { $testmain = $false ; }

  if ($testmode)
  { $max_filesize = 1000000 ; }
  else
  { $max_filesize = 55*1024*1024 ; }

  # for huge English database intermediate files will be used and data will be sorted in four chunks
  # extra file I/O amply compensated by less memory swapping
  $save_memory = $false ;

  if ($language eq "en")
  { $save_memory = $true ; }
  else
  {
   if ($testmode) # test splitting with small non english Wikipedia
   { $split_files = $true ; }
  }

  $default_table_alignment = $true ;
  $edit_tables = $false ;

  if ($mark_tables)
  {
#   $mark_table_start = "<br>»»<br>" ;
#   $mark_table_end   = "««<br>" ;
    $mark_table_start = "<ul>" ;
    $mark_table_end   = "</ul>" ;
  }
  else
  {
    $mark_table_start = "" ;
    $mark_table_end   = "" ;
  }

  $author = "Erik Zachte" ;
  $mail   = "epzachte\@chello.nl" ;
  $site   = "http://members.chello.nl/epzachte/Wikipedia" ;

  if ($language eq "nl")
  {
    $title_mainpage = "Hoofdpagina" ;
    $symbol_mainpage = "H" ;
    $license  = "GNU Vrije Documentatie Licentie" ;
    $imagetag = "Afbeelding" ;
    $colophon  = "Database geëxporteerd op:<br>&nbsp;&nbsp;<b>*#1#*</b><br>" .
                "Conversie datum:<br>&nbsp;&nbsp;<b>" . &GetDate (time) . "</b><br>Inhoud:<br>&nbsp;&nbsp;*#2#*<p>" .
                "Dit document mag vrij gedistribueerd worden onder de voorwaarden van de <a>$license</a><hr>" .
                "Wikipedia online: http://nl.wikipedia.org<p>" .
                "Rechtstreeks naar online versie van een artikel:<br>" .
                "http://nl.wikipedia.org/wiki/<i>artikelnaam</i><p>" .
                "Of ga naar de hoofdpagina en gebruik de zoekfunctie.<hr>" .
                "Wikipedia NL maakt deel uit van Wikipedia, een gemeenschapsproject " .
                "met als doel in elke taal een complete encyclopedie op het web te creëren. " .
                "Iedereen kan Wikipedia gebruiken om informatie te zoeken én om informatie toe te voegen. " .
                "Om mee te helpen artikelen te schrijven, corrigeren en uit te breiden is geen aanmelding nodig, " .
                "u kunt meteen aan de slag.<hr>" .
                "Perl conversie script $version<p>" .
                "Script ontwikkeld door $author<p>Amsterdam<br>Mail: $mail<br>Site: $site" ;

  }

  if ($language eq "de")
  {
    $title_mainpage = "Hauptseite" ;
    $symbol_mainpage = "H" ;
    $license  = "GNU Freie Documentationslizenz" ;
    $imagetag = "Bild" ;
    $colophon  = "Database exportiert am:<br>&nbsp;&nbsp;<b>*#1#*</b><br>" .
                "Konvertierungsdatum:<br>&nbsp;&nbsp;<b>" . &GetDate (time) . "</b><br>Inhalt:<br>&nbsp;&nbsp;*#2#*<p>" .
                "Die Inhalte der Wikipedia unterliegen der <a>$license</a>, das bedeutet sie sind frei und werden es für immer bleiben.<hr>".
                "Wikipedia online: http://www.wikipedia.de<p>" .
                "Direkt zur online Version des Artikels:<br>" .
                "http://www.wikipedia.de/wiki/<i>Artikel</i><p>" .
                "Oder gehe zur Hauptseite und benutze die Suchfunktion.<hr>" .
                "Wikipedia ist eine frei verfügbare Enzyklopädie in Wikiform, ".
                "deren Beiträge gemeinsam erarbeitet werden. " .
                "Sie ist der deutschsprachige Bereich der internationalen Wikipedia. " .
                "Jeder kann hier mitarbeiten und Wissen beitragen, es ist keine Anmeldung nötig! " .
                "Es geht auch ganz einfach." .
                "Wer will kann sofort zu beliebigen Themen Neues schreiben oder vorhandene Artikel verbessern!<hr>" .
                "Perl Script $version<p>" .
                "Script geschrieben von $author<p>Amsterdam<br>Mail: $mail<br>Site: $site" ;

  }

  if ($language eq "en")
  {
    $title_mainpage = "Main Page" ;
    $symbol_mainpage = "M" ;
    $license  = "GNU Free Documentation License" ;
    $imagetag = "image" ;
    $colophon  = "Database exported on:<br>&nbsp;&nbsp;<b>*#1#*</b><br>" .
                "Conversion date:<br>&nbsp;&nbsp;<b>" . &GetDate (time) . "</b><br>Content:<br>&nbsp;&nbsp;*#2#*<p>" .
                "This document can be distributed freely under the <a>$license</a><hr>" .
                "*#3#*" .
                "Wikipedia online: http://www.wikipedia.org<p>" .
                "Go directly to an article:<br>" .
                "http://www.wikipedia.org/<i>article</i><p>" .
                "You can also use the online search function.<hr>" .
                "Perl conversion script $version<br><br>" .
                "Script written by $author<p>Amsterdam, The Netherlands<br>mail: $mail<br>site: $site" ;
  }

  if ($language eq "eo")
  {
    $title_mainpage = "CXefpagxo" ;
    $symbol_mainpage = "CX" ;
    $license  = "GFDL Permesilo de GNU por Liberaj Dokumentoj" ;
    $imagetag = "Dosiero" ;
    $colophon  = "Datumbazo eksportita je:<br>&nbsp;&nbsp;<b>*#1#*</b><br>" .
                "Konvertita dato:<br>&nbsp;&nbsp;<b>" . &GetDate (time) . "</b><br>Content:<br>&nbsp;&nbsp;*#2#*<p>" .
                "Cxi tiu dokumento povas esti distribuata libere sub la <a:GFDL>$license</a><hr>" .
                "Vikipedio rete: http://www.wikipedia.org<p>" .
                "Iru rekte al artikolo:<br>" .
                "http://eo.wikipedia.org/<i>artikolo</i><p>" .
                "Vi povas ankaux uzi la retan sercxilon.<hr>" .
                "Perl konverta skripto $version<br><br>" .
                "Skripto programita de $author<p>Amsterdamo<br>Nederlando<p>Retposxto: $mail<br>Retpagxaro: $site" ;
  }

  if ($language eq "fr")
  {
    $title_mainpage = "Accueil" ;
    $symbol_mainpage = "A" ;
    $license  = "Licence de documentation libre GNU" ;
    $imagetag = "image" ;
    $colophon  = "Base de données exportée:<br>&nbsp;&nbsp;<b>*#1#*</b><br>" .
                "Date de conversion:<br>&nbsp;&nbsp;<b>" . &GetDate (time) . "</b><br>Contenu:<br>&nbsp;&nbsp;*#2#*<p>" .
                "Ce document peut être distribué librement sous <a>$license</a><hr>" .
                "Wikipédia en ligne : http://fr.wikipedia.org<p>" .
                "Aller directement à un article :<br>" .
                "http://fr.wikipedia.org/<i>article</i><p>" .
                "Vous pouvez aussi faire une recherche en ligne.<hr>" .
                "Script de conversion Perl $version<br><br>" .
                "Script écrit par $author<p>Amsterdam, Les Pays Bas<br>E-mail: $mail<br>Site web: $site" ;
  }

  if ($remove_html)
  { $colophon =~ s/\&nbsp\;/ /gi ; }

  # select subset of articles for small demo database
  # @demo  will list all titles for articles that contain $demo_filter
  # @demo2 will list all titles for articles that link directly to a article in @demo
  $demo_selection_1 = $false ;
  $demo_filter      = "Kofi Annan" ;

  # select even smaller subset of articles by title for demo database
  $demo_selection_2 = $false ;
  if ($demo)
  { $demo_selection_2 = $true ; }

  foreach $symbol (&HTMLsymbols)
  {
    ($ascii, $html, $html2) = split(/\s\s*/,$symbol);
     if ((index ($html, '?') == -1) && # stores &...; codes only
         (index ($html, '#') == -1))   # convert numeric codes directly
     { $asciicodes .= $ascii . $html ; }
  }

  foreach $symbol (&ASCIIapproximations)
  {
    ($html, $ascii) = split(/\s\s*/,$symbol);
    @asciicodes2 {$html} = $ascii ;
  }

  foreach $symbol (&ASCIIapproximationsEsperanto)
  {
    ($html, $ascii) = split(/\s\s*/,$symbol);
    @asciicodesEsperanto {$html} = $ascii ;
  }

  $ImageExtensions = "(gif|jpg|png|bmp|jpeg)";
  $article_size_max = 0 ;
  $articles = 0 ;
  $redirects = 0 ;
  &Phase1 ;
  &Phase2 ;
  &Phase3 ;
  exit ;

sub Phase1 {
  $timestart = time ;
  $timestartjob = time ;
  $file_in  = "cur_table_" .$language . ".sql" ;

  if ($palm)
  { $target = "_PALM" ; }
  elsif ($epoc)
  { $target = "_EPOC" ; }
  else
  { $target = "_(P)PC" ; }
  
  if (! $split_files) # else output filename(s) will be determined later
  { $file_out = "WP_" . uc($language) . $target . ".tab" ; }
  if (($demo_selection_1) ||
      ($demo_selection_2))
  { $file_out = "WP_" . uc($language) . $target . "_DEMO.tab" ; }

  if (! -e $file_in)
  {
    if (-e "cur_table.sql")
    {
      print ("Sql dump file '" . $file_in . "' not found.\n") ;
      $file_in = "cur_table.sql" ;
      print ("Sql dump file '" . $file_in . "' found. Will use this one.\n") ;
    }
  }
  if (! -e $file_in)
  { print ("Sql dump file '" . $file_in . "' not found or in use!\nExecution aborted") ; exit  }
  open "FILE_IN", "<", $file_in  ;

  if (! $split_files) # else output filename(s) will be determined later
  {
    if (! open "FILE_OUT", ">", $file_out)
    { print "\nOutput file \'" . $file_out . "\'could not be opened!\nExecution aborted" ; exit ; }
  }

  $filesize = -s $file_in ;
  $fileage  = -M $file_in ;

  if (($palm) && ($filesize > $max_filesize))
  { $split_files = $true ; }

  if (($demo_selection_1) ||
      ($demo_selection_2))
  { $split_files = $false ; }

  if ($split_files)
  { $save_memory = $true ; }

  if ($save_memory)
  { open "FILE_TEMP_1",  ">", "WikiToTomeTemp.1" ; }

  print "\nPHASE 1:\n" ;
  print "Read sql dump file \'" . $file_in . "\' (". sprintf ("%.1f", $filesize/(1024*1024)) . " Mb).\n" ;

  print "Extract and edit articles.\n" ;
  if ($split_files)
  { print "Split database in several volumes for Palm users.\n" ;}
  print "Relevant data extracted (Mb):\n" ;

  $mb_entries   = 0 ;
  $size_entries = 0 ;

  while (($line = <FILE_IN>) and (index ($line, "INSERT INTO cur VALUES") == -1))
  { ; }

  &ProcessSqlBlock ;
  while (($line = <FILE_IN>)  && (length ($line) > 1))
  {
    if (($testmode) && (++$sql_blocks_processed == 5))
    { last ; }
    if ($testmain && $mainpage_found)
    { last ; }
    &ProcessSqlBlock ;
  }

  close "FILE_IN" ;
  if ($save_memory)
  { close "FILE_TEMP_1" ; }

  print sprintf ("%.1f", $size_entries/(1024*1024)) . " Mb\n" ;

  print "Longest article is " . $article_size_max . " bytes\n" ;

  if ($split_files)
  {

    if ($testmode)
    {
      # show total article size per letter
      for ($v = 0 ; $v < 26 ; $v++)
      { print chr ($v + ord ("A")) . ": " . @size_per_letter [$v] . "\n" ; }
    }

    # implement size constraint per volume:
    #   determine which article should go to which volume,
    #   based on first letter of title
    $letter1 = "A" ;
    $size = 0 ;
    $sizetot = 0 ;
    for ($l = 0 ; $l <= 26 ; $l++)
    {
      if (($l == 26) || ($l > 0) && ($size + @size_per_letter [$l] > $max_filesize))
      {
        $letter2  = chr ($l-1 + ord ("A")) ;
        $ord1     = ord ($letter1) - ord ("A") ;
        $ord2     = ord ($letter2) - ord ("A") ;
        for ($l2 = $ord1 ; $l2 <= $ord2 ; $l2++)
        { @volumes [$l2] = $letter1 . $letter2 ; }
        if ($testmode)
        { print $letter1 . "-" . $letter2 . ": " . $size . "\n" ; }
        $letter1  = chr ($l   + ord ("A")) ;
        $sizetot += $size ;
        $size = @size_per_letter [$l] ;
      }
      else
      { $size += @size_per_letter [$l] ; }
    }

    if ($testmode)
    {
      # show distribution of letter per volume
      print "A-Z: " . $sizetot . "\n" ;
      for ($l = 0 ; $l < 26 ; $l++)
      { print $l . ": " . @volumes [$l] . "\n" ; }
    }
  }
  print "Phase 1 execution took " . mmss (time - $timestart). ".\n" ;
}

sub Phase2 {
  $timestart = time ;

  print "\nPHASE 2:\n" ;

  if ($save_memory)
  { open "FILE_TEMP_1", "<", "WikiToTomeTemp.1" ; }

  if ($demo_selection_1)
  {
    print "Select articles for demo database\n" ;
    print "Articles processed (x1000) :\n" ;
    $articles = 0 ;

    if ($save_memory)
    {
      while ($entry = <FILE_TEMP_1>)
      {
        chomp $entry ;
        ($title, $article) = split ('\|\|', $entry, 2) ;
        &ScanLinks ;
        &ShowProgress (++$articles) ;
      }
    }
    else
    {
      foreach $entry (@entries)
      {
        ($title, $article) = split ('\|\|', $entry, 2) ;
        &ScanLinks ;
        &ShowProgress (++$articles) ;
      }
    }
    print "\n\n" ;

    if ($save_memory)
    {
      close "FILE_TEMP_1" ;
      open "FILE_TEMP_1", "<", "WikiToTomeTemp.1" ;
    }
  }

  print "Remove links to non-existing articles\n" ;
  print "Articles processed (x1000) :\n" ;

  if ($save_memory)
  {
    open "FILE_TEMP_2a", ">", "WikiToTomeTemp.2a" ;
    open "FILE_TEMP_2b", ">", "WikiToTomeTemp.2b" ;
    open "FILE_TEMP_2c", ">", "WikiToTomeTemp.2c" ;
    open "FILE_TEMP_2d", ">", "WikiToTomeTemp.2d" ;
  }

  $articles = 0 ;
  if ($save_memory)
  {
    while ($entry = <FILE_TEMP_1>)
    {
      chomp $entry ;
      &ProcessEntryPhase1 ;
    }
  }
  else
  {
    while ($#entries >= 0)
    {
      $entry = pop @entries ;
      &ProcessEntryPhase1 ;
    }
  }

  print "\nNumber of articles written is "  . $articles . "\n" ;
  print "Number of redirects written is " . $redirects . "\n" ;
  print "Number of links to redirects replaced by actual link is " . $links_replaced . "\n" ;
  if ($split_files)
  {
    print "Number of links to external volume is " . $links_external . "\n" ;
    print "Number of links to external volume that needed explanation is " . $links_explained . "\n" ;
  }
  print "Phase 2 execution took " . mmss (time - $timestart). ".\n" ;

  if (defined $title_mainpage)
  {
    if ($language eq "nl")
    {
      $title = "  Nederlandse Wikipedia" ; # two spaces make it first entry in the index
      $colophon =~ s/\*\#1\#\*/&GetDate (time - int ($fileage*24*60*60)) /e ;
      $colophon =~ s/\*\#2\#\*/<b>$articles<\/b> artikelen, <b>$redirects<\/b> verwijzingen, <b>$valid_links<\/b> hyperlinks/ ;
    }
    if ($language eq "de")
    {
      $title = "  Deutsche Wikipedia" ; # two spaces make it first entry in the index
      $colophon =~ s/\*\#1\#\*/&GetDate (time - int ($fileage*24*60*60)) /e ;
      $colophon =~ s/\*\#2\#\*/<b>$articles<\/b> Artikel, <b>$redirects<\/b> Aliassen, <b>$valid_links<\/b> Hyperlinks/ ;
    }
    if ($language eq "fr")
    {
      $title = "  Wikipédia française" ; # two spaces make it first entry in the index
      $colophon =~ s/\*\#1\#\*/&GetDate (time - int ($fileage*24*60*60)) /e ;
      $colophon =~ s/\*\#2\#\*/<b>$articles<\/b> articles, <b>$redirects<\/b> redirections, <b>$valid_links<\/b> liens/ ;
    }
    if ($language eq "en")
    {
      $title = "  English Wikipedia" ; # two spaces make it first entry in the index
      $colophon =~ s/\*\#1\#\*/&GetDate (time - int ($fileage*24*60*60)) /e ;
      if ($split_files)
      {
        $links_clickable = $valid_links - $links_external ;
        $colophon =~ s/\*\#2\#\*/<b>$articles<\/b> articles, <b>$redirects<\/b> aliasses, <b>$links_clickable<\/b> hyperlinks \+ <b>$links_external<\/b> external references/ ;
        $palmtext = "Palm version: due to Palm OS or TomeRaider constraints the encyclopedia had to be split into several volumes.<br>" .
                    "<b>$links_external<\/b> hyperlinks were converted to non-clickable external references.<p>" .
                    "Hyperlinks (clickable) are shown with dotted underline.<br>" .
                    "External references (non-clickable) are shown with continuous underline, like <u>color</u> or <u>red*</u>.<br>" .
                    "<u>color<\/u> : This text can be found in the index of the volume containing letter C.<br>" .
                    "<u>red*<\/u> : At the end of the article the index entry for this article is listed, e.g.<br>" .
                    "red => Color red<hr>" ;
        $colophon =~ s/\*\#3\#\*/$palmtext/ ;
      }
      else
      {
        $colophon =~ s/\*\#2\#\*/<b>$articles<\/b> articles, <b>$redirects<\/b> aliasses, <b>$valid_links<\/b> hyperlinks/ ;
        $colophon =~ s/\*\#3\#\*//
      } ;
    }
    if ($language eq "eo")
    {
      $title = "  Vikipedio en Esperanto" ; # two spaces make it first entry in the index
      $colophon =~ s/\*\#1\#\*/&GetDate (time - int ($fileage*24*60*60)) /e ;
      $colophon =~ s/\*\#2\#\*/<b>$articles<\/b> artikoloj/ ;
    }

    $entry = $title . "\t" . $colophon ;
    $introduction = $entry ;

    if ($remove_html)
    {
      $article =~ s/<font[^>]*>//gi ;
      $article =~ s/<\/font[^>]*>//gi ;
    }
  }

  if ($save_memory)
  {
    close "FILE_TEMP_1" ;
    close "FILE_TEMP_2a" ;
    close "FILE_TEMP_2b" ;
    close "FILE_TEMP_2c" ;
    close "FILE_TEMP_2d" ;
    unlink "WikiToTomeTemp.1" ;
  }

}

sub Phase3 {
  $timestart = time ;
  print "\nPHASE 3:\n" ;
  if (! $split_files)
  { print "Sort data and write TomeRaider input file \'" . $file_out . "\'\n" ; }
  else
  { print "Sort data and write TomeRaider input files. \n" ; }
  if ($language eq "en")
  { print "Please be patient\n" ; }

  if ((! $split_files) && (defined ($mainpage)))
  {
    print FILE_OUT $introduction . "\n" ;
    $title   = $title_mainpage ;
    $article = $mainpage ;
    &ProcessEntryPhase2 ;
    print FILE_OUT " " . $title_mainpage . "\t" . $article . "\n" ;
  }

  $mb_entries   = 0 ;
  $size_entries = 0 ;

  if ($save_memory)
  { &Phase3a ; }
  else
  { &Phase3b ; }

  close "FILE_OUT" ;
  if ($split_files)
  {
    close "FILE_OUT_ALIAS" ;
    close "FILE_OUT_LARGE" ;
  }

  print "\nPhase 3 execution took " .  mmss (time - $timestart). ".\n\n" ;
  print "Complete execution took " . mmss (time - $timestartjob). ".\n\n" ;

  if ($testmode)
  {
    foreach $tag (keys %tags)
    
    { print $tag . ": " . @tags {$tag} . "\n"; }
  }
  
  if ($testmode2) # build html table of unicodes for which no substitute exists yet
  {
    open "FILE_OUT", ">", "Unicode.html"  ;
    print FILE_OUT "<html><head></head><body><table width=600>\n" ;
    print FILE_OUT "<tr><th align='left'>glyph</th><th align='right'> html</th><th align='right'> ascii</th><th align='right'> hex</th><th align='right'> decimal</th><th align='right'> count</th></tr>\n" ;
    @unicodekeys = keys %unicodes ;
    @unicodekeys = sort {@unicodes {$b} <=> @unicodes {$a}} @unicodekeys ;
    foreach $unicode (@unicodekeys)
    {
      $htmlcode = &UnicodeToHtmlTag ($unicode) ;
      $htmltag  = "&amp;#" . substr ($htmlcode,2,length ($htmlcode)-3) . ";" ;
      $line = "<tr><td>" . $htmlcode .
             "</td><td align='right'>" . $htmltag .
             "</td><td align='right'>" . $unicode .
             "</td><td align='right'>" ;
      for ($c = 0 ; $c < length ($unicode) ; $c++)
      { $line .= sprintf ("%X", ord (substr ($unicode,$c,1))) ; }
      $line .= "</td><td align='right'>" ;
      for ($c = 0 ; $c < length ($unicode) ; $c++)
      {
        if ($c > 0)
        { $line .= " " ; }
        $line .= ord (substr ($unicode,$c,1)) ;
      }
      $line .= "</td><td align='right'>" .@unicodes {$unicode} . "</td></tr>\n" ;
      print FILE_OUT $line ;
      if (@unicodes {$unicode} < 10)
      { last ; }
    }
    print FILE_OUT "</table></body>\n" ;
    close "FILE_OUT" ;
    
  }
  if (defined (@errors))
  {
    open "FILE_ERR", ">", "WikiToTomeErrors_$language.txt" ;
    print FILE_ERR @errors ;
    close "FILE_ERR" ;
  }

  if ($false) # debug code
  {
    @orphanedkeys = keys %orphaned ;
    @orphanedkeys = sort { $a cmp $b} @orphanedkeys ;
    open "FILE_LINKS", ">", "LinksOrphaned.txt"  ;
   foreach $link (@orphanedkeys)
    { print FILE_LINKS $link ."-" . @orphaned {$link} . "\n" ; }
    close "FILE_LINKS" ;
  
    open "FILE_LINKS", ">", "LinksExisting.txt"  ;
    foreach $link (@titles)
    { print FILE_LINKS $link . "\n" ; }
    close "FILE_LINKS" ;
  
    @redirectedkeys = keys %titles_redirected ;
    @redirectedkeys = sort { $a cmp $b} @redirectedkeys ;
    open "FILE_LINKS", ">", "LinksRedirected.txt"  ;
    foreach $link (@redirectedkeys)
    { print FILE_LINKS $link . "\n" ; }
    close "FILE_LINKS" ;
  }
}

sub Phase3a {
  print "\nPHASE 3.1: Sort entries !-c\n" ;
  Phase3a_sort ("a") ;
  print "\nPHASE 3.2: Sort entries d-j\n" ;
  Phase3a_sort ("b") ;
  print "\nPHASE 3.3: Sort entries k-p\n" ;
  Phase3a_sort ("c") ;
  print "\nPHASE 3.4: Sort entries q-ÿ\n" ;
  Phase3a_sort ("d") ;
  unlink "WikiToTomeTemp.2a" ;
  unlink "WikiToTomeTemp.2b" ;
  unlink "WikiToTomeTemp.2c" ;
  unlink "WikiToTomeTemp.2d" ;
}

sub Phase3a_sort
{
  $set = shift ;
  undef @titles ;
  undef @entries2 ;
  $ndx = -1 ;

  open "FILE_TEMP_2", "<", "WikiToTomeTemp.2" . $set ;
  while ($entry = <FILE_TEMP_2>)
  {
    chomp $entry ;
    ($title) = split ('\|\|', $entry, 2) ;
    push (@titles, lc ($title) . "\n" . '||' . ++$ndx) ;
    push (@entries2, $entry) ;
  }
  close "FILE_TEMP_2" ;

  @titles = sort {$a cmp $b} @titles ;

  print "Sort complete.\n" ;
  print "Writing to output file\n" ;
  print "Data written (Mb):\n" ;

  foreach $title (@titles)
  {
    ($dummy, $ndx) = split ('\|\|', $title) ;
    ($title, $article) = split ('\|\|', @entries2 [$ndx], 2) ;
    if ($article eq "") { next ; }
    $entry = $title . "\t" . $article ;

    &write_entry ($entry) ;

    $size_entries += length ($entry) ;
    if ($size_entries > ($mb_entries + 1) * 1024 * 1024)
    {
      print ++$mb_entries . " " ;
      if ($mb_entries % 20 == 0)
      { print "\n" ; }
    }
  }
}

sub Phase3b
{
  undef @titles ;
  $ndx = -1 ;
  foreach $entry (@entries2)
  {
    ($title) = split ('\|\|', $entry, 2) ;
    push (@titles, lc ($title) . "\n" . '||' . ++$ndx) ;
  }

  @titles = sort {$a cmp $b} @titles ;

  print "Sort complete\n" ;
  print "Writing to output file\n" ;
  print "Data written (Mb):\n" ;

  foreach $title (@titles)
  {
    ($dummy, $ndx) = split ('\|\|', $title) ;
    ($title, $article) = split ('\|\|', @entries2 [$ndx], 2) ;
    if ($article eq "") { next ; }
    $entry = $title . "\t" . $article ;

    &write_entry ($entry) ;

    $size_entries += length ($entry) ;
    if ($size_entries > ($mb_entries + 1) * 1024 * 1024)
    {
      print ++$mb_entries . " " ;
      if ($mb_entries % 20 == 0)
      { print "\n" ; }
    }
  }
}

sub write_entry
{
  my $entry = shift ;

  if (! $split_files)
  {
    if ((! $testmain) || $mainpage_found)
    { print FILE_OUT $entry . "\n" ; }
  }
  else
  {
    $volume = @volumes [&TitleToIndex ($title)] ;
    if ($volume ne $prev_volume)
    {
      if (defined ($prev_volume))
      { close "FILE_OUT" ; }

      if ($palm)
      { $target = "_PALM" ; }
      elsif ($epoc)
      { $target = "_EPOC" ; }
      else
      { $target = "_(P)PC" ; }
      
      $file_out = "WP_" . uc($language) . $target . "_" . $volume . ".tab" ;
      if (! open "FILE_OUT", ">", $file_out)
      { print "\nOutput file \'" . $file_out . "\'could not be opened!\nExecution aborted" ; exit ; }

      $introduction2 = $introduction ;
      $introduction2 =~ s/Wikipedia/Wikipedia - Volume $volume/ ;
      if ((substr ($volume,0,1) gt "G") || (substr ($volume,1,1) lt "G"))
      { $introduction2 =~ s/<a>($license)<\/a>/<font color="#008800">$1<\/font>/ } ;
      print FILE_OUT $introduction2 . "\n" ;

      # add main page to every volume
      # make sure hyperlinks are converted correctly for this volume
      if (defined ($mainpage))
      {
        $article = $mainpage ;
        $title = $volume ;
        &ProcessEntryPhase2 ;
        $save_entry = $entry ;
        print FILE_OUT " " . $title_mainpage . "\t" . $article . "\n" ;
        $entry = $save_entry ;
      }
      $prev_volume = $volume ;
    }
    if ((! $testmain) || $mainpage_found)
    { print FILE_OUT $entry . "\n" ; }
  }
}

sub ProcessSqlBlock
{
  #temporary replace all \' text quotes, leaving only CSV quotes
  $line =~ s/\\\'/\#\*\$\@/g ;

  #restore double quotes \" -> "
  $line =~ s/\\\"/"/g ;

  $first = "\\(" ;        # find start of record = opening brace
  $text  = "'([^']*)'," ; # alphanumeric field (save contents between quotes)
  $text2 = "'[^']*'," ;   # alphanumeric field
  $int   = "\\d+," ;      # integer field
  $float = "[^,]*," ;     # used for floating point field
  $last  = "[^)]*\\)" ;   # last field and closing brace
  while ($line =~ m/$first$int($int)$text$text$text2$int$text2$text2$text2$int$int$int$int$float$last/go)
  {
    $namespace = $1 ;
    if ($namespace ne '0,') { next ; } # keep 'real' articles, skip meta stuff
    $title     = $2 ;
    $article   = $3 ;

    if ($testmain)
    {
      if ($mainpage_found) { last ; }
      if ($language eq "eo")
      { $title =~ s/\xC4\x88efpa\xC4\x9Do/CXefpagxo/ ; }

      $title_mainpage2 = $title_mainpage ;
      $title_mainpage2 =~ s/ /_/g ;
      if ($title ne $title_mainpage2)
      { next ; }
      $mainpage_found = $true ;
    }
    
    # select a few specific articles for demo database ?
    if ($demo_selection_2)
    {
      $title_lc = lc ($title) ;
      if (($title_lc ne "gold") &&
          ($title_lc ne "rembrandt") &&
          ($title_lc ne "periodic_table") &&
          ($title_lc ne "nobel_prize_in_physics") &&
          ($title_lc ne "geography") &&
          ($title_lc ne "new_york") &&
          ($title_lc ne "list_of_lists") &&
          ($title_lc ne "list_of_reference_tables") &&
          ($title_lc ne "gnu_free_documentation_license") &&
          ($title_lc ne "main_page"))
      { next ; }

      if (($palm) || ($epoc))
      {

        if ($title_lc eq "periodic_table") # messy without html
        { next ; }
      }
    }

    if (index ($title, "Start Tobak") != -1) # ugly empty page in Eng WP just in front
    { next ; }
    
    # first remove links to other wikipedias
    # unicode characters may otherwise be translated into ascii and cause trouble
    $article =~ s/\[\[..\:[^\]]*\]\]//g ;

    # replace Wikipedia variables with dummy text
    $article =~ s/\{\{[^\}]*\}\}/\.\./g ;

    #remove html comments
    $article =~ s/\<\!\-\-.*?\-\-\>//g ;

    # determine longest article
    $l = length ($article) ;
    if ($l > $article_size_max)
    { $article_size_max = $l ; }

    if (($title eq "") || ($article eq ""))
    { next ; }

    # really crappy page with lots of broken unicodes
    if (($title eq "Unu_Minuto_Vikio") && ($language eq "eo"))
    { next ; }

    $articles++ ;

    $entry = $title . "||" . $article . "\n" ;

    # unicode -> html character codes &#nnnn;
    if ($language eq "eo")
    { $entry =~ s/([\x80-\xFF]+)/&UnicodeToHtml($1)/ge ; }

    # html character tags -> ascii
    $entry =~ s/([AEIOUaeiou])\&\#768\;/&$1grave;/g ;   # $#768 = 'combining grave'
    $entry =~ s/\&\#768\;/&#96;/g ;
    $entry =~ s/([AEIOUYaeiouy])\&\#769\;/&$1acute;/g ; # $#769 = 'combining acute'
    $entry =~ s/\&\#769\;/&acute;/g ;
    $entry =~ s/([AEIOUaeiou])\&\#770\;/&$1circ;/g ;    # $#770 = 'combining circumflex'
    $entry =~ s/\&\#770\;/^/g ;
    $entry =~ s/([AONaon])\&\#771\;/&$1tilde;/g ;       # $#771 = 'combining tilde'
    $entry =~ s/\&\#771\;/~/g ;
    $entry =~ s/([AEIOUaeiouy])\&\#776\;/&$1uml;/g ;    # $#776 = 'combining diaeresis'
    $entry =~ s/\&\#776\;/&uml;/g ;

    $entry =~ s/(\&[^\;]{2,6}\;)/&HtmlToAscii($1)/ge ;

    if (index ($entry, "<nowiki>") != -1)
    {
      @segments = split ("<nowiki>", $entry) ;
      foreach $segment (@segments)
      {
        ($nowiki, $remainder) = split ("</nowiki>", $segment) ;
        if (defined ($remainder))
        { $segment = &NoWiki($nowiki) . $remainder ; }
      }
      $entry = join ("", @segments) ;
    }

    # restore quotes
    $entry =~ s^\#\*\$\@^'^g ;

    ($title,$article) = split ('\|\|', $entry, 2) ;

    # html character tags -> ascii double char approximation
    $title_original = $title ;
    $title =~ s/(\&[^\;]{2,6}\;)/&HtmlToAsciiApproximation($1)/ge ;

    if (($palm) || ($epoc))
    { $article =~ s/(\&[^\;]{2,6}\;)/&HtmlToAsciiApproximation($1)/ge ; }

    if ((! ($palm || $epoc)) && ($title_original ne $title))
    { $article = "'''(" . $title_original . ")'''<p>" . $article ; }
    
    # Dutch Wikipedia:
    # - remove counter tag from main page
    # - remove introduction + remark about number of articles (moved to first entry)
    # - remove current events (not so current anymore)
    # - remove meta links (links to [[Wikipedia:...]]) meta info not in this db
    # - after all this editing the table of contents shows well even on a small screen
    if (($title eq "Hoofdpagina") && ($language eq "nl"))
    {
      $search = "'''Exacte wetenschappen" ;
      if (index ($article, $search) != -1)
      { $article = substr ($article, index ($article, $search)) ; }
      $search = "[[Jaaroverzichten]]" ;
      if (index ($article, $search) != -1)
      { $article = substr ($article, 0, index ($article, $search) + length ($search)) ; }
      $article =~ s/<\/?td>//g ;
      $article =~ s/\'\'\'\[\[Speciaal\:Allpages\|Bladeren\]\]\'\'\'/<b>Bladeren<\/b>/ ;
      $article =~ s/\[[^\]]*Speciaal\:Allpages[^\]]*\]\s*//g ;
      $article =~ s/\'\'\'Onlangs overleden\'\'\'.*?\'\'\'/<br><br>\'\'\'/ ;
      $article =~ s/- \[\[Overzichten/\[\[Overzichten/g ;
      $article =~ s/\'\s*<br>/\': /g ;
      $article =~ s/\\r\\n//g ;
    }

    # German Wikipedia:
    if (($title eq "Hauptseite") && ($language eq "de"))
    {
      $search = "'''\[\[Wissenschaft" ;
      if (index ($article, $search) != -1)
      { $article = substr ($article, index ($article, $search)) ; }
      $search = "KFZ-Kennzeichen]]" ;
      if (index ($article, $search) != -1)
      { $article = substr ($article, 0, index ($article, $search) + length ($search)) ; }
      $article =~ s/<[\/]?(table|td|tr|th)[^\>]*>//g ;
      $article =~ s/<div[^>]*>/<p>/g ;
      $article =~ s/\[\[\.\..*?\\r\\n// ;
      $article =~ s/-\\r\\n<\/div/\\r\\n<\/div/ ;
    }

    # French Wikipedia:
    if (($title eq "Accueil") && ($language eq "fr"))
    {
      $search = "\'\'\'Sciences naturelles et mathématiques" ;
      if (index ($article, $search) != -1)
      { $article = substr ($article, index ($article, $search)) ; }
      $search = "Biographies]]" ;
      if (index ($article, $search) != -1)
      { $article = substr ($article, 0, index ($article, $search) + length ($search)) ; }
      $article =~ s/<[\/]?(table|td|tr|th)[^\>]*>//g ;
      $article =~ s/<font[^>]*>[^<]*<\/font>/<p>/g ;
      $article =~ s/\\r\\n\\r\\n/ /g ;
      $article =~ s/\~/-/g ;
      $article =~ s/(\'\'\'.*?\'\'\')/$1:/g ;
      $article =~ s/<br>//g ;
    }

    # Esperanto Wikipedia:
    if (($title eq "CXefpagxo") && ($language eq "eo"))
    {
      $article =~ s/<em .*?<\/em>//g ;
      if (index ($article, "<h3") != -1)
      { $article = substr ($article, index ($article, "<h3")) ; }
      $search = "Tagoj]]" ;
      if (index ($article, $search) != -1)
      { $article = substr ($article, 0, index ($article, $search) + length ($search)) ; }
      if (! ($palm || $epoc))
      { $article = "'''(\&\#264;efpa\&\#285;o)'''<p>". $article ; }
      $article =~ s/\=\=\=([^=]*)\=\=\=/<p><b>$1<\/b>/g ;
      $article =~ s/<h\d.*?>(.*?)<\/h\d>/<p><b>$1<\/b>:/g ;
    }

    # English Wikipedia: edit main page
    # original article is a two column table with lots of info which is not so
    # relevant on a handheld
    # - remove current events (not so current anymore)
    # - remove sections Writing Articles and About the Project
    # - after all this editing the table of contents shows well even on a small screen
    if (($title eq "Main_Page") && ($language eq "en"))
    {
      $article =~ s/and are currently working on \.\./and are already working on more than 160,000/ ;
      $article =~ s/Visit the \[\[Wikipedia\:Help\|help page\]\] and experiment in the \[\[Wikipedia\:Sandbox\|sandbox\]\] to/Visit http:\/\/www.wikipedia.org and/ ;
      $search = "===Mathematical and Natural Sciences" ;
      if (index ($article, $search) != -1)
      { $article = substr ($article, index ($article, $search)) ; }
      $search = "How-to]]s" ;
      if (index ($article, $search) != -1)
      { $article = substr ($article, 0, index ($article, $search) + length ($search)) ; }
      $article =~ s/\[\[Special.*?-//g ;
      if ($true) # ($palm || $epoc)
      {
        $article =~ s/\\r\\n/ /g ;
        $article =~ s/\=\=\=([^=]*)\=\=\=/<p><b>$1<\/b>:/g ;
        $article =~ s/<p>// ;
      }
    }

    $title_lc = lc ($title) ;
    $title_lc =~ s/_/ /g ;

#   if ($article =~ m/\#redirect\s*\[\[/i)
    if ($article =~ m/redirect/i)
    {
      ($dummy, $link)  = split ('\[\[', $article) ;
      ($link,  $dummy) = split ('\]\]', $link) ;
      $redirect = $title ;
      $redirect =~ s/_/ /g ;
      $link     =~ s/_/ /g ;
      @titles_redirected {$redirect} = $link ;
    }
    else
    { @titles {$title_lc} = 1 ; }

    # collect subsets of articles for demo database (optional)
    if ($demo_selection_1)
    {
      if ($article =~ /$demo_filter/i)
      { @demo {$title_lc} = 1 ; }
    }

    # just to save some valuable space (handheld device):
    # turn links to any 4 digit year or other small number into ordinary text
    # $article =~ s/\[\[([\d]{1,4})\]\]/$+/g ;

    # turn cr,lf into <br>
    $article =~ s/\\r\\n/<br>/g ;
    $article =~ s/\\n/<br>/g ;

    # change wiki lists into html lists
    if (! ($article =~ /\#redirect/i))
    { $article = WikiLinesToHtml ($article) ; }
    else
    { $article =~ s/('*)'''(.*?)'''/$1<b>$2<\/b>/g; }


    #remove images
    $article =~ s/\[\[$imagetag:([^\]]*)\]\]//gi ;
    $article =~ s/(http:|https:|ftp:)[^ ]+\.$ImageExtensions//g ;

    #edit horizontal ruler
    $article =~ s/\<br\>----+\<br\>/<hr>/g ;

    # convert underscores to spaces in $title
    $title =~ s/\_/ /g ;

    # remove linebrakes (<br>) inside table(s) except within <td>..</td>
    # remove single linebreaks (<br>) outside (nested) table(s),
    #                                except within <pre> .. </pre> block
    $article =~ s /<br>/*|br|*/gi ; # prevent split on <br>
    @segments = split ('<', $article) ;

    undef @td_depth ;
    $in_pre_block = $false ;
    @td_depth [0] = 0 ;

    $ready = $false ;
    while (! $ready)
    {
      $ready = $true ;

      # detect empty table (probably contained only images which now have been stripped)
      # detect table with just one text (probably contained image + explanation)
      $ndx_empty_table_begin = -1 ;
      $ndx_empty_table_end   = -1 ;
      $ndx   = -1 ;
      foreach (@segments)
      {
        $ndx++ ;

        # detect <pre> .. </pre> block
        if (/^pre\>/i)
        { $in_pre_block = $true ; }
        if (/^\/pre\>/i)
        { $in_pre_block = $false ; }

        if (/^font/i)
        {
          # update existing font size info (optional)
          if ($tables_small_font)
          { $_ =~ s/ size\=[ ]*[^ ]*([^>]*)>/ size=\"2\"$1>/i ; }
        }

        # also mark top level tables in non obtrusive way (will be marked as such later)
        if (/^table/i)
        {
          # remove existing table width info (optional)
          if ($tables_small_font)
          {
            $_ =~ s/ width\=[ ]*[^ ]* / /i ;
            $_ =~ s/ width\=[ ]*[^>]*>/>/i ;
          }

          ++$#td_depth ;
          if ($#td_depth == 1)
          { $_ =~ s/table/TaBlE/i ; } # table is not a nested table
          $ndx_empty_table_begin = $ndx ;
          $non_empty_table_segments = 0 ;
        }
        if (/^\/table/i)
        {
          if ($#td_depth > 0) # safeguard against invalid sequence
          {--$#td_depth ;}
          if ($#td_depth == 0)
          { $_ =~ s/table/TaBlE/i ; } # table was not a nested table
          if ($ndx_empty_table_begin != -1)
          {
            $ndx_empty_table_end = $ndx ;
            last ;
          }
        }
        if ((/^\/tr/i) ||
            (/^tr/i))
        {
          if (@td_depth [$#td_depth] > 0) # fix table: add missing </td>
          { $_ = "/td><" . $_ ; }
        }
        if (/^td/i)
        {
          if (@td_depth [$#td_depth] > 0) # fix table: add missing </td>
          { $_ = "/td><" . $_ ; }
          else
          { ++@td_depth [$#td_depth] ; }
        }
        if (/^\/td/i)
        { --@td_depth [$#td_depth] ; }

        if (@td_depth [$#td_depth] > 0) # keep linebreaks within <td>...</td>
        {
          $_ =~ s/\*\|br\|\*/<br>/gi ;
          $_ =~ s/<br><br>/<br>/gi ;
        }
        else
        {
          if ($#td_depth > 0)           # still inside table ?
          { $_ =~ s/\*\|br\|\*//gi ; }  # remove other linebreaks inside table
        }
        if ($#td_depth > 0) # remove <pre>, </pre> within tables
        { $_ =~ s/^[\/]?pre\>/PrE>/g; } # mark <pre> , </pre> for removal
        else
        {
          if (! $in_pre_block)
          {
            $_ =~ s/\*\|br\|\*\*\|br\|\*/<p>/gi ; # double linebreaks are significant
            $_ =~ s/\*\|br\|\*/ /gi ;             # remove all remaining single linebreaks
          }
        }
        if ($ndx_empty_table_begin != -1)
        {
          $segment = $_ ;
          $segment =~ s/<[^>]*>[\s]*//gi ;
          $segment =~ s/[^>]*>[\s]*//gi ;
          if ($segment ne "")
          {
            if (++$non_empty_table_segments > 1)
            { $ndx_empty_table_begin = - 1 ; }
          }
        }
      }
      if (($ndx_empty_table_begin != -1) &&
          ($ndx_empty_table_end   != -1))
      {
        # print "\nTable removed:" . $title . ", line " . $ndx_empty_table_begin . "-" . $ndx_empty_table_end "\n" ;
        $remove_segments = $ndx_empty_table_end - $ndx_empty_table_begin + 1 ;
        splice (@segments, $ndx_empty_table_begin, $remove_segments) ;
        $ready = $false ;
      }
    }
    $article = join ('<', @segments) ;

    # restore all remaining linebreaks
    $article =~ s/\*\|br\|\*/<br>/gi ;

    if ($tables_small_font)
    {
      $article =~ s/<(td|th)([^>]*)>/<$1$2><font size="2">/gi ;
      $article =~ s/<\/(td|th)([^>]*)>/<\/font><\/$1$2>/gi ;
      $article =~ s/<caption([^>]*)>/<caption$1><font size="2"><b>/gi ;
      $article =~ s/<\/caption([^>]*)>/<\/b><\/font><\/caption$1>/gi ;
    }

    # remove <pre> </pre> tags from within tables (were marked as PrE)
    $article =~ s/<br><[\/]?PrE><br>/<br>/g ;
    $article =~ s/<[\/]?PrE>//g ;

    # remove some insignificant linebreaks within <td>...</td>
    $article =~ s/(<td[^>]*>)<br>/$1/gi ;
    $article =~ s/<br><\/td>/<\/td>/gi ;

    # convert html table -> ascii table (optional)
    if ($edit_tables)
    { $article =~ s/<[\/]?t[h|r|d][^\>]*>/<br>/gi ; }

    # remove most linebreaks
    $article =~ s/(<br>|<p>)(<li>|<dd>|<[\/]?[d|o|u]l>)[\s]*/$2/gi ;

    # remove linebreak before <h2>,<h3>,etc and after </h2>,</h3>,etc
    $article =~ s/(<p>|<br>)(<[\/]?h[\d]>)[\s]*/$2/gi ;

    # remove redundant double quotes (not needed in explorer engine that TomeRaider uses)
    $article =~ s/(border|cellpadding|cellspacing|colspan)="(\d)"/$1=$2/gi ;

    # TomeRaider PC version handles table alignment correct
    # TomeRaider Pocket PC version doesn't ->
    # revert to default table alignment
    if ($default_table_alignment)
    {
      if (index ($article, '<') != -1)
      {
        @segments = split ('<', $article) ;
        foreach (@segments)
        {
          if (/^table/i) # (lc (substr ($segment, 0, 5)) eq 'table')
          {
            $_ =~ s/ align=[ ]*\"[^\"]*\"//i ;
            $_ =~ s/ align=[ ]*\'[^\']*\'//i ;
            $_ =~ s/ align=[ ]*[^ ]* / /i ;
            $_ =~ s/ align=[ ]*[^\>]*\>/>/i ;
          }
        }
        $article = join ('<', @segments) ;
      }
    }

    # determine longest article
    $l = length ($article) ;
    if ($l > $article_size_max)
    { $article_size_max = $l ; }

    if ((length ($title) > 0) && (length ($article) > 1)) # last char is \n -> length >= 1
    {
      # add title in lowercase (needed for sort, TomeRaider key sequence is case insensitive)
      # otherwise lc function would have to be performed on each comparison within sort routine
      $entry = $title . "||" . $article ;
      chomp ($entry) ;

      @size_per_letter [&TitleToIndex ($title)] += length ($entry) ;

      if ($save_memory)
      { print FILE_TEMP_1 $entry . "\n" ; }
      else
      { @entries [++$#entries] = $entry ; }
    }

    $size_entries += length ($entry) ;
    if ($size_entries > ($mb_entries + 1) * 1024 * 1024)
    {
      print ++$mb_entries . " " ;
      if ($mb_entries % 20 == 0)
      { print "\n" ; }
    }
  }
}

sub ProcessEntryPhase1
{
  ($title, $article) = split ('\|\|', $entry, 2) ;

  $title_lc = lc ($title) ;
  if ($demo_selection_1)
  {
    # skip articles that are not marked for the demo database
    if (! ((defined (@demo  {$title_lc})) ||
           (defined (@demo2 {$title_lc}))))
    { next ; }
  }

  # skip redirect when article with same title (case insensitive) is present
  if (($article =~ /redirect/i) && (defined @titles {$title_lc}))
  { }
  else
  {
    $article_redirect = $false ;
    if ($title eq $title_mainpage)
    { $mainpage = $article ; }
    &ProcessEntryPhase2 ;
    if ($entry ne "")
    {
      if ($save_memory)
      {
        $ch = lc (substr ($entry,0,1)) ;
        if ($ch lt 'd')
        { print FILE_TEMP_2a $entry . "\n" ; }
        elsif ($ch lt 'k')
        { print FILE_TEMP_2b $entry . "\n" ; }
        elsif ($ch lt 'q')
        { print FILE_TEMP_2c $entry . "\n" ; }
        else
        { print FILE_TEMP_2d $entry . "\n" ; }
      }
      else
      { @entries2 [++$#entries2] = $entry ; }

      if ($article_redirect)
      { ++$redirects ; }
      else
      { ++$articles ; }

      $records = $redirects + $articles ;
      if ($records % 1000 == 0)
      {
        print $records / 1000 . " " ;
        if ($records % 20000 == 0)
        { print "\n" ; }
      }
    }
  }
}

sub ProcessEntryPhase2
{
  $volume_article = @volumes [&TitleToIndex ($title)] ;

  #edit redirects
  if ($article =~ /\#redirect/i)
  {
    $article = substr ($article, 0, index (lc ($article), "]]")+2) ;
    $article =~ s/\#redirect/\>/gi ;
    $article_redirect = $true ;
  }

  #edit hyperlinks, remove underscores in links
  undef @references ;
  if (index ($article ,'[[') != -1)
  {
    @segments = split ('\[\[', $article) ;
    for ($i = 1 ; $i <= $#segments ; $i++)
    {
      if (index (@segments [$i], ']]') != -1)
      {
        undef $show ;
        undef $remainder ;
        ($link, $remainder) = split (']]', @segments [$i], 2) ;
        $link =~ s/_/ /g ;

        if (index ($link, "|") != -1)
        {
          ($search, $show) = split ('\|', $link) ;
          if ($search eq $show)
          { undef $show ; }
        }
        else
        {
          $search = $link ;
          # link = like [[animal]]s -> make whole word shown as hyperlink
          if ($remainder =~ /^[a-z]{1,3}([^a-z]|$)/)
          {
            $suffix = $remainder ;
            $suffix =~ s/(^[a-z]+).*/$1/ ;
            $show = $search . $suffix ;
            $remainder = substr ($remainder, length ($suffix)) ;
          }
        }
        # replace &#nnn; html codes in links by ascii 2 byte approximations
        # for Palm both title and entry have been converted already
        # for Pocket PC only title hs been converted, so link should follow
        # (TomeRaider for Pocket PC can display &$nnn; tags properly, but
        #  fails to do so in index, besides manual input of search string
        #  makes simpler format necessary)
        if (! ($palm || $epoc))
        {
          $search_original = $search ;
          $search =~ s/(\&[^\;]{2,6}\;)/&HtmlToAsciiApproximation($1)/ge ;
          if ($search ne $search_original)
          {
            # actual link now is always different from displayed text
            if (! defined ($show))
            { $show = $search_original ; }
          }
        }
        
        # might as well replace aliasses (redirected strings)
        $iterations = 0 ;
        do
        {
          $searchnew = @titles_redirected {$search} ;
          if (defined ($searchnew)) # alias exists ?
          {
            if (defined (@titles {lc ($searchnew)})) # and really points to article ?
            {
              if ($search eq $searchnew) # circular reference
              {  undef ($searchnew) ; }
              else
              {
                # print $title . ": " . $search . " -> " . $ searchnew . "\n";
                if (! defined ($show))
                { $show = $search ; }
                $search = $searchnew ;
                ++ $links_replaced ;
              }
            }
          }
        }
        while ((defined ($searchnew)) && (++ $iterations < 5)) ;

        $found = $false ;
        $mark_external = $false ; # entry in other file (Palm only, files are split < 32 Mb)
        if ($demo_selection_1)
        {
          if ((defined (@demo  {lc($search)})) ||
              (defined (@demo2 {lc($search)})))
          { $found = $true ; }
        }
        else
        {
          # find link in @titles = list of real articles
          # or in @titles_redirected = list of redirect articles
          # latter search only needed when link text equals displayed text
          # otherwise redirection to real article title has already been resolved
          if ((defined (@titles {lc($search)})) ||
             ((! defined ($show)) && (@titles_redirected {$search})))
          {
            if (! $split_files)
            { $found = $true ; }
            else
            {
              # title found, but will it be in the same file ?
              $volume_search = @volumes [&TitleToIndex ($search)] ;
              if ($volume_article eq $volume_search)
              { $found = $true ; }
              else
              {
                $mark_external = $true ;
                $links_external ++ ;
              }
            }
          }
          if (($found) || ($mark_external))
          { ++ $valid_links ; }
        }

        if ($demo_selection_1) # retain all links in demo
        { $found = $true ; }

        if (! $found)
        {
          @orphaned {$search} ++ ;
          if (defined $show) # hyperlink is different from visible text
          {

            $link = "!" . $show . "!" ;
            if ($mark_external)
            {
              # are link and visible text almost the same ?
              # like in [[monkey|monkeys]] or vice versa
              # if so no need to add explicit reference, mark green
              # if not add explicit reference at end of article and mark violet
              if ((index ($show,   $search) != -1) ||
                  (index ($search, $show)   != -1))
              #{ $link = "<ufont color='\#008800'>" . $link . "</font>"; }
              { $link = "<u>" . $link . "</u>"; }
              else
              {
                # $link = "<font color='\#CC0000'>" . $link . "</font>";
                $link = "<u>" . $link . "*</u>";
                push (@references, $show . " => " . $search) ;
                $links_explained ++ ;
              }
            }
          }
          else
          {
            $link = "!" . $search . "!" ;
            if ($mark_external)
            # { $link = "<font color='\#008800'>" . $link . "</font>" ; }
            { $link = "<u>" . $link . "</u>" ; }
          }
        }
        else
        {
          if (defined $show)
          { $link = $search . "|" . $show ; }
          else
          { $link = $search ; }
        }

        if (defined ($remainder))
        { @segments [$i] = $link . "]]" . $remainder ; }
        else
        { @segments [$i] = $link ; }
      }
    }
    $article = join ('[[', @segments) ;
    $article =~ s/\[\[(<[^>]*>)?!([^\!]*)!(\*)?(<[^>]*>)?\]\]/$1$2$3$4/g ;
    if (defined (@references))
    {
      @references = sort { lc($a) cmp lc($b) } @references ;
      $prevref    = "" ;
      $article   .= "<hr>" ;
      for ($r = 0 ; $r <= $#references ; $r++)
      {
        if (@references [$r] ne $prevref)
        {
          $article .= @references [$r] ;
          $prevref  = @references [$r] ;
          if ($r < $#references)
          { $article .= "<br>" ; }
        }
      }
    }
  }

  # remove redirects to non existing articles
  if ($article_redirect)
  {
    if  (! ($article =~ /\[\[/))
    {
      $entry = "" ;
      return ;
    }
  }

  #edit hyperlinks [[show|search]] -> <a:search>show</a>
  $article =~ s/\[\[([^\]]*)\|([^\]]*)\]\]/\<a\:$1\>$2\<\/a\>/g ;
  $article =~ s/\[\[([^\]]*)\]\]/\<a\>$+\<\/a\>/g ;

  # tables are only place where double linebreaks are wanted
  $article =~ s/<TaBlE([^\>]*)>/<br><br><TaBlE$1>/g ;
  $article =~ s/<\/TaBlE([^\>]*)>/<\/TaBlE$1><br><br>/g ;

  # remove <div ...> .. </div>
  $article =~ s/<(\/)?div[^>]*>//gi ;

  # replace <var ...> .. </var>
  $article =~ s/<(\/)?var[^>]*>/$1i/gi ;

  # remove yet more redundant linebreaks
  $article =~ s/\s*<br>\s*<p>/<p>/gi ;
  $article =~ s/\s*<p>\s*<br>/<p>/gi ;
  $article =~ s/(\s*\<p\>)+/<p>/g ;
  $article =~ s/^[ ]*<br><br>//i ;
  $article =~ s/^[ ]*<br>//i ;
  $article =~ s/^[ ]*<p>//i ;
  $article =~ s/(<\/dl>|<\/ol>|<\/ul>|<\/h\d>)[ ]*<br>[ ]*<br>/$1/gi ;
  $article =~ s/(<\/dl>|<\/ol>|<\/ul>|<\/h\d>)[ ]*<br>/$1/gi ;

  # remove attributes on some html tags
  $article =~ s/<p[^>]*>/<p>/gi ;
  $article =~ s/<hr[^>]*>/<hr>/gi ;
  $article =~ s/<h([\d])[^>]*>/<h$1>/gi ;

  # TomeRaider uses tab character as delimiter between title and article
  $article =~ s/\t/ /g ;

  # remove leading and trailing spaces
  $title =~ s/^[\s]*//g ;
  $title =~ s/[\s]*$//g ;
  $article =~ s/^[\s]*//g ;
  $article =~ s/[\s]*$//g ;

  if (($title eq "") || ($article eq ""))
  {
    $entry = "" ;
    return ;
  }

  # remove font info (optional)
  if ($remove_font_info)
  {
    $article =~ s/<font[^>]*>//gi ;
    $article =~ s/<\/font[^>]*>//gi ;
  }

  if ($remove_html)
  { &RemoveHtmlCode ; }

  # reset table tag of top level tables
  $article =~ s/TaBlE/table/g ;

  $entry = $title . "||" . "[<a:" . $title_mainpage . ">" . $symbol_mainpage . "</a>] " . $article ;
  
  # force proper sort sequence
  # "ABC" sorts after "ABC D"
  # "ABC\n" sorts before "ABC D\n"
}

sub RemoveHtmlCode
{
  # remove definition list tags
  $article =~ s/<dl>\s*<dt>\s*<dd>/<p>/gi ; # empty <dt> part often used for indentation
  $article =~ s/<dl>//gi ;
  $article =~ s/<dt>//gi ;
  $article =~ s/<dd>/<br>- /gi ;
  $article =~ s/<\/dl>//gi ;

  # remove <pre> </pre> tags
  $article =~ s/<[\/]?pre>//gi ;

  if (index ($article, "<TaBlE") != -1)
  {
    # remove <br> <p> from within tables
    @segments = split ("<TaBlE", $article) ;
    foreach (@segments)
    {
      if (index ($_, "<\/TaBlE") != -1)
      {
        @segments2 = split ("<\/TaBlE", $_) ;
        @segments2 [0] =~ s/<br>/ /gi ;
        @segments2 [0] =~ s/<p>/ /gi ;
        $_ = join ("<\/TaBlE", @segments2) ;
      }
    }
    $article = join ("<TaBlE", @segments) ;
  }

  # mark start/end of top level tables (= not nested) (optional)
  if ($mark_tables)
  {
    $article =~ s/(<br>)?<TaBlE[^\>]*>/$mark_table_start/g ;
    $article =~ s/<\/TaBlE[^\>]*>(<br>)?/$mark_table_end/g ;
  }
  $article =~ s/<table[^\>]*>/<ul>/gi ;
  $article =~ s/<\/table[^\>]*>/<\/ul>/gi ;
# $article =~ s/<table[^\>]*>/<br>/gi ;
# $article =~ s/<\/table[^\>]*>/<\/ul>/gi ;

  $article =~ s/(<caption[^>]*>)(.*?)(<\/caption[^>]*>)/$1<b>$2<\/b>$3/gi ;
  $article =~ s/<\/caption[^>]*>/<br>/gi ;
  $article =~ s/<caption[^>]*>//gi ;

  $article =~ s/(<th[^>]*>)(.*?)(<\/th[^>]*>)/$1<b>$2<\/b>$3/gi ;
  $article =~ s/<\/th[^>]*>[\s]*<\/tr/<\/tr/gi ;
  $article =~ s/<\/th[^>]*>/ - /gi ;
  $article =~ s/<th[^>]*>//gi ;

  $article =~ s/<\/td[^>]*>[\s]*<\/tr/<\/tr/gi ;
  $article =~ s/<\/td[^>]*>/ - /gi ;
  $article =~ s/<td[^>]*>//gi ;

  $article =~ s/<\/tr[^>]*>//gi ;
  $article =~ s/<tr[^>]*>/<li>/gi ; #
  $article =~ s/<li><br><li>/<li>/gi ; #
  $article =~ s/<br><\/ul>/<\/ul>/gi ; #
  $article =~ s/<li><\/ul>/<\/ul>/gi ; #
  $article =~ s/<li><ul>/<ul>/gi ; #
  $article =~ s/<li><li>/<li>/gi ; #

  # remove font (= also color) info
  $article =~ s/<font[^>]*>//gi ;
  $article =~ s/<\/font[^>]*>//gi ;

  # remove not TRML text formatting info
  $article =~ s/<br[^>]*>/<br>/gi ;
  $article =~ s/<(\/)?small>//gi ;
  $article =~ s/<(\/)?code>//gi ;
  $article =~ s/<(\/)?tt>//gi ;
  $article =~ s/<(\/)?sup>//gi ;
  $article =~ s/<(\/)?sub>//gi ;
  $article =~ s/<(\/)?left>//gi ;
  $article =~ s/<(\/)?center>//gi ;
  $article =~ s/<(\/)?right>//gi ;
  $article =~ s/<\/p>//gi ;
  $article =~ s/<\/li//gi ;
  $article =~ s/<(\/)?math//gi ;
  $article =~ s/<s>/[/gi ;
  $article =~ s/<\/s>/]/gi ;

  $article =~ s/<h[\d]>/<p><b>/gi ;
  $article =~ s/<\/h[\d]>/<\/b>/gi ;

  $article =~ s/<blockq(uo|ou)te>/»/gi ;
  $article =~ s/<\/blockq(uo|ou)te>/«/gi ;

  $article =~ s/<em[^>]*>(.*?)<\/em[^>]*>/<b>$1<\/b>/gi ;
  $article =~ s/<strong[^>]*>(.*?)<\/strong[^>]*>/<b>$1<\/b>/gi ;
  $article =~ s/<cite[^>]*>(.*?)<\/cite[^>]*>/<i>$1<\/i>/gi ;

  $article =~ s/<(\/)?ol>/<$1ul>/gi ;

  # now again remove some redundant <br>'s
  $article =~ s/<br>[\s]*<p>/<p>/gi ;
  $article =~ s/<p>[\s]*<br>/<p>/gi ;
  $article =~ s/(<br>){2,7}/<p>/gi ;
  $article =~ s/(<p>){2,7}/<p>/gi ;
  $article =~ s/<br>[\s]*<p>/<p>/gi ;
  $article =~ s/<p>[\s]*<br>/<p>/gi ;
  $article =~ s/»»(<p>){1,7}/»»<br>/gi ;
  $article =~ s/»»(<br>){2,7}/»»<br>/gi ;
  $article =~ s/(<p>){1,7}««/<br>««/gi ;
  $article =~ s/(<br>){2,7}««/<br>««/gi ;
  $article =~ s/(<p>|<br>)?[\s]*<hr>[\s]*(<p>|<br>)?/<hr>/gi ;
  $article =~ s/^(<p>|<br>)*// ;

  # replace html characters
  $article =~ s/\&nbsp\;/ /gi ;
  $article =~ s/\&\#8211\;/-/gi ;     # long dash -> dash
  $article =~ s/\&\#822(0|1)\;/"/gi ; # special quote -> double quote
  $article =~ s/\&\#821(6|7)\;/'/gi ; # special quote -> single quote
  $article =~ s/\&\#8226\;/º/gi ;     # dot
  $article =~ s/(\[)?(\()?\&\#91\;(\))?(\])?/[/g ;
  $article =~ s/(\[)?(\()?\&\#93\;(\))?(\])?/]/g ;
  $article =~ s/\&\#16412;/"/g ;      # opening double quote
  $article =~ s/\&\#16413;/"/g ;      # closing double quote
  $article =~ s/\&\#16422;/.../g ;    # elipsis

  $article =~ s/\&([\d]{2,6})\;/*/g ; # numeric html tags  &#nnn;
  $article =~ s/\&([^\;]{2,10})\;/[$1]/g ; # named html tags e.g. &Alpha;

  # collect not supported html tags that still exist (will be printed)
  if ($testmode)
  {
    $notags = $article ;
    $notags =~ s/<\!\-\-(.*?)\-\->/$1/gi ;
    $notags =~ s/<(\/)?a[^>]*>//gi ;
    $notags =~ s/<-/-/gi ;
    $notags =~ s/->/-/gi ;
    $notags =~ s/<(\/)?b>//gi ;
    $notags =~ s/<(\/)?i>//gi ;
    $notags =~ s/<(\/)?u>//gi ;
    $notags =~ s/<(\/)?ul>//gi ;
    $notags =~ s/<(\/)?ol>//gi ;
    $notags =~ s/<li>//gi ;
    $notags =~ s/<br>//gi ;
    $notags =~ s/<p>//gi ;
    $notags =~ s/<hr>//gi ;
    $notags =~ s/(<[^\s][^>]*>)/&CntTag($1)/ge ;
  }
}

sub CntTag
{
  $tag = shift ;
  if (($tag ne "<=>") && (substr ($tag,0,2) ne "<>"))
  { @tags {$tag} ++ ; }
}

sub HTMLsymbols {
	return (
	"\!  ?          &#33;",
	"\"  &quot;     &#34;",
	"\#  ?          &#35;",
	"$   ?          &#36;",
	"%   ?          &#37;",
# "&   &amp;      &#38;",
	"'   ?          &#39;",
	"(   ?          &#40;",
	")   ?          &#41;",
	"*   ?          &#42;",
	"+   ?          &#43;",
	",   ?          &#44;",
	"-   ?          &#45;",
	".   ?          &#46;",
	"/   ?          &#47;",
	":   ?          &#58;",
	";   ?          &#59;",
	"<   &lt;       &#60;",
	"=   ?          &#61;",
	">   &gt;       &#62;",
	"?   ?          &#63;",
	"©   &copy;     &#64;",
	"[   ?          &#91;",
	"\\  ?          &#92;",
	"]   ?          &#93;",
	"^   ?          &#94;",
	"_   ?          &#95;",
	"`   ?          &#96;",
	"{   ?          &#123;",
	"|   ?          &#124;",
	"}   ?          &#125;",
	"~   ?          &#126;",
	"    ?          &#160;",
	"¡   &iexcl;    &#161;",
	"¢   &cent;     &#162;",
	"£   &pound;    &#163;",
	"¤   &curren;   &#164;",
	"¥   &yen;      &#165;",
	"¦   &brvbar;   &#166;",
	"§   &sect;     &#167;",
	"¨   &uml;      &#168;",
	"©   &copy;     &#169;",
	"ª   &ordf;     &#170;",
	"«   &laquo;    &#171;",
	"¬   &not;      &#172;",
	"­   &shy;      &#173;",
	"®   &reg;      &#174;",
	"¯   &macr;     &#175;",
	"°   &deg;      &#176;",
	"±   &plusmn;   &#177;",
	"²   &sup2;     &#178;",
	"³   &sup3;     &#179;",
	"´   &acute;    &#180;",
	"µ   &micro;    &#181;",
	"¶   &para;     &#182;",
	"·   &middot;   &#183;",
	"¸   &cedil;    &#184;",
	"¹   &sup1;     &#185;",
	"º   &ordm;     &#186;",
	"»   &raquo;    &#187;",
	"¼   &frac14;   &#188;",
	"½   &frac12;   &#189;",
	"¾   &frac34;   &#190;",
	"¿   &iquest;   &#191;",
	"®   &reg;",
	"À   &Agrave;   &#192;",
	"Á   &Aacute;   &#193;",
	"Â   &Acirc;    &#194;",
	"Ã   &Atilde;   &#195;",
	"Ä   &Auml;     &#196;",
	"Å   &Aring;    &#197;",
	"Æ   &AElig;    &#198;",
	"Ç   &Ccedil;   &#199;",
	"È   &Egrave;   &#200;",
	"É   &Eacute;   &#201;",
	"Ê   &Ecirc;    &#202;",
	"Ë   &Euml;     &#203;",
	"Ì   &Igrave;   &#204;",
	"Í   &Iacute;   &#205;",
	"Î   &Icirc;    &#206;",
	"Ï   &Iuml;     &#207;",
	"Ð   &ETH;      &#208;",
	"Ñ   &Ntilde;   &#209;",
	"Ò   &Ograve;   &#210;",
	"Ó   &Oacute;   &#211;",
 	"Ô   &Ocirc;    &#212;",
	"Õ   &Otilde;   &#213;",
	"Ö   &Ouml;     &#214;",
	"×   &times;    &#215;",
	"Ø   &Oslash;   &#216;",
	"Ù   &Ugrave;   &#217;",
	"Ú   &Uacute;   &#218;",
	"Û   &Ucirc;    &#219",
	"Ü   &Uuml;     &#220;",
	"Ý   &Yacute;   &#221;",
	"Þ   &THORN;    &#222;",
	"ß   &szlig;    &#223;",
	"à   &agrave;   &#224;",
	"á   &aacute;   &#225;",
	"â   &acirc;    &#226;",
	"ã   &atilde;   &#227;",
	"ä   &auml;     &#228;",
	"å   &aring;    &#229;",
	"æ   &aelig;    &#230;",
	"ç   &ccedil;   &#231;",
	"è   &egrave;   &#232;",
	"é   &eacute;   &#233;",
	"ê   &ecirc;    &#234;",
	"ë   &euml;     &#235;",
	"ì   &igrave;   &#236;",
	"í   &iacute;   &#237;",
	"î   &icirc;    &#238;",
	"ï   &iuml;     &#239;",
	"ð   &eth;      &#240;",
	"ñ   &ntilde;   &#241;",
	"ò   &ograve;   &#242;",
	"ó   &oacute;   &#243;",
	"ô   &ocirc;    &#244;",
	"õ   &otilde;   &#245;",
	"ö   &ouml;     &#246;",
	"÷   &divide;   &#247;",
	"ø   &oslash;   &#248;",
	"ù   &ugrave;   &#249;",
	"ú   &uacute;   &#250;",
	"û   &ucirc;    &#251;",
	"ü   &uuml;     &#252;",
	"ý   &yacute;   &#253;",
	"þ   &thorn;    &#254;",
	"ÿ   &yuml;     &#255;")
}

sub ASCIIapproximationsEsperanto {
	return (
	"&#265; cx", # c+circumflex  Esperanto x-based notation
	"&#264; CX", # C+circumflex  Esperanto x-based notation
	"&#285; gx", # g+circumflex  Esperanto x-based notation
	"&#284; GX", # G+circumflex  Esperanto x-based notation
	"&#293; hx", # h+circumflex  Esperanto x-based notation
	"&#292; HX", # H+circumflex  Esperanto x-based notation
	"&#309; jx", # j+circumflex  Esperanto x-based notation
	"&#308; JX", # J+circumflex  Esperanto x-based notation
	"&#349; sx", # s+circumflex  Esperanto x-based notation
	"&#348; SX", # S+circumflex  Esperanto x-based notation
	"&#365; ux", # u+breve       Esperanto x-based notation
	"&#364; UX") # U+breve       Esperanto x-based notation
}

sub ASCIIapproximations {
	return (

# 256- 535 are rude (sometimes very !) approximations of accented and unusual characters
# certainly open to improvement
# see the originals ? write a html file as follows:
#  open "FILE_OUT", ">", "Unicode.html"  ;
#  print FILE_OUT "<html><head></head><body>\n" ;
#  for ($c = 255 ; $c < 535; $c++)
#  { print FILE_OUT "\&amp;#" . $c . "; -> \&#" . $c . "; <br>\n" ; }
#  print FILE_OUT "</body>\n" ;
#  close "FILE_OUT" ;

  "&#256; A",
  "&#257; a",
  "&#258; A",
  "&#259; a",
  "&#260; A",
  "&#261; a",
  "&#262; C",
  "&#263; c",
  "&#264; C",
  "&#265; c",
  "&#266; C",
  "&#267; c",
  "&#268; C",
  "&#269; c",
  "&#270; D",
  "&#271; d",
  "&#272; D",
  "&#273; d",
  "&#274; E",
  "&#275; e",
  "&#276; E",
  "&#277; e",
  "&#278; E",
  "&#279; e",
  "&#280; E",
  "&#281; e",
  "&#282; E",
  "&#283; e",
  "&#284; G",
  "&#285; g",
  "&#286; G",
  "&#287; g",
  "&#288; G",
  "&#289; g",
  "&#290; G",
  "&#291; g",
  "&#292; H",
  "&#293; h",
  "&#294; H",
  "&#295; h",
  "&#296; I",
  "&#297; i",
  "&#298; I",
  "&#299; i",
  "&#300; I",
  "&#301; i",
  "&#302; I",
  "&#303; i",
  "&#304; I",
  "&#305; i",
  "&#306; IJ",
  "&#307; ij",
  "&#308; J",
  "&#309; j",
  "&#310; K",
  "&#311; k",
  "&#312; k",
  "&#313; L",
  "&#314; l",
  "&#315; L",
  "&#316; l",
  "&#317; L",
  "&#318; l",
  "&#319; L",
  "&#320; I",
  "&#321; L",
  "&#322; l",
  "&#323; N",
  "&#324; n",
  "&#325; N",
  "&#326; n",
  "&#327; N",
  "&#328; n",
  "&#329; n",
  "&#330; N",
  "&#331; n",
  "&#332; O",
  "&#333; o",
  "&#334; O",
  "&#335; o",
  "&#336; O",
  "&#337; o",
  "&#338; OE",
  "&#339; oe",
  "&#340; R",
  "&#341; r",
  "&#342; R",
  "&#343; r",
  "&#344; R",
  "&#345; r",
  "&#346; S",
  "&#347; s",
  "&#348; S",
  "&#349; s",
  "&#350; S",
  "&#351; s",
  "&#352; S",
  "&#353; s",
  "&#354; T",
  "&#355; t",
  "&#356; T",
  "&#357; t",
  "&#358; T",
  "&#359; t",
  "&#360; U",
  "&#361; u",
  "&#362; U",
  "&#363; u",
  "&#364; U",
  "&#365; u",
  "&#366; U",
  "&#367; u",
  "&#368; U",
  "&#369; u",
  "&#370; U",
  "&#371; u",
  "&#372; W",
  "&#373; w",
  "&#374; Y",
  "&#375; y",
  "&#376; Y",
  "&#377; Z",
  "&#378; z",
  "&#379; Z",
  "&#380; z",
  "&#381; Z",
  "&#382; z",
  "&#383; I",
  "&#384; b",
  "&#385; B",
  "&#386; b",
  "&#387; b",
  "&#388; b",
  "&#389; b",
  "&#390; C",
  "&#391; C",
  "&#392; c",
  "&#393; D",
  "&#394; D",
  "&#395; d",
  "&#396; d",
  "&#397; d",
  "&#398; E",
  "&#399; e",
  "&#400; e",
  "&#401; F",
  "&#402; f",
  "&#403; G",
  "&#404; Y",
  "&#405; h",
  "&#406; I",
  "&#407; I",
  "&#408; K",
  "&#409; k",
  "&#410; t",
  "&#411; L",
  "&#412; M",
  "&#413; N",
  "&#414; e",
  "&#415; t",
  "&#416; O",
  "&#417; o",
  "&#418; S",
  "&#419; s",
  "&#420; P",
  "&#421; p",
  "&#422; R",
  "&#423; S",
  "&#424; s",
  "&#425; S",
  "&#426; I",
  "&#427; t",
  "&#428; T",
  "&#429; t",
  "&#430; T",
  "&#431; U",
  "&#432; u",
  "&#433; O",
  "&#434; N",
  "&#435; Y",
  "&#436; y",
  "&#437; Z",
  "&#438; z",
  "&#439; 3",
  "&#440; 3",
  "&#441; 3",
  "&#442; 3",
  "&#443; 2",
  "&#444; 5",
  "&#445; 5",
  "&#446; ¿",
  "&#447; p",
  "&#448; I",
  "&#449; II",
  "&#450; \#",
  "&#451; !",
  "&#452; DZ",
  "&#453; Dz",
  "&#454; dz",
  "&#455; LJ",
  "&#456; Lj",
  "&#457; lj",
  "&#458; NJ",
  "&#459; Nj",
  "&#460; nj",
  "&#461; A",
  "&#462; a",
  "&#463; I",
  "&#464; i",
  "&#465; O",
  "&#466; o",
  "&#467; U",
  "&#468; u",
  "&#469; U",
  "&#470; u",
  "&#471; U",
  "&#472; u",
  "&#473; U",
  "&#474; u",
  "&#475; U",
  "&#476; u",
  "&#477; e",
  "&#478; A",
  "&#479; a",
  "&#480; A",
  "&#481; a",
  "&#482; AE",
  "&#483; ae",
  "&#484; G",
  "&#485; g",
  "&#486; G",
  "&#487; g",
  "&#488; K",
  "&#489; k",
  "&#490; Q",
  "&#491; q",
  "&#492; Q",
  "&#493; q",
  "&#494; 3",
  "&#495; 3",
  "&#496; j",
  "&#497; DZ",
  "&#498; Dz",
  "&#499; dz",
  "&#500; G",
  "&#501; g",
  "&#502; ?",
  "&#503; ?",
  "&#504; ?",
  "&#505; ?",
  "&#506; A",
  "&#507; a",
  "&#508; AE",
  "&#509; ae",
  "&#510; O",
  "&#511; o",
  "&#512; A",
  "&#513; a",
  "&#514; A",
  "&#515; a",
  "&#516; E",
  "&#517; e",
  "&#518; E",
  "&#519; e",
  "&#520; I",
  "&#521; i",
  "&#522; i",
  "&#523; i",
  "&#524; O",
  "&#525; o",
  "&#526; O",
  "&#527; o",
  "&#528; R",
  "&#529; r",
  "&#530; R",
  "&#531; r",
  "&#532; U",
  "&#533; u",
  "&#534; U",
  "&#535; u",

	"&#771; ã",
# intermediate step &#aaa; will be converted to [aaa] anyway

  "&#913; &Alpha;" ,   # greek capital letter alpha,  U0391
  "&#914; &Beta;" ,    # greek capital letter beta,  U0392
  "&#915; &Gamma;" ,   # greek capital letter gamma,  U0393
  "&#916; &Delta;" ,   # greek capital letter delta,  U0394
  "&#917; &Epsilon;" , # greek capital letter epsilon,  U0395
  "&#918; &Zeta;" ,    # greek capital letter zeta,  U0396
  "&#919; &Eta;" ,     # greek capital letter eta,  U0397
  "&#920; &Theta;" ,   # greek capital letter theta,  U0398
  "&#921; &Iota;" ,    # greek capital letter iota,  U0399
  "&#922; &Kappa;" ,   # greek capital letter kappa,  U039A
  "&#923; &Lambda;" ,  # greek capital letter lambda,  U039B
  "&#924; &Mu;" ,      # greek capital letter mu,  U039C
  "&#925; &Nu;" ,      # greek capital letter nu,  U039D
  "&#926; &Xi;" ,      # greek capital letter xi,  U039E
  "&#927; &Omicron;" , # greek capital letter omicron,  U039F
  "&#928; &Pi;" ,      # greek capital letter pi,  U03A0
  "&#929; &Ro;" ,      # greek capital letter rho,  U03A1
  "&#931; &Sigma;" ,   # greek capital letter sigma,  U03A3
  "&#932; &Tau;" ,     # greek capital letter tau,  U03A4
  "&#933; &Upsilon;" , # greek capital letter upsilon,  U03A5
  "&#934; &Phi;" ,     # greek capital letter phi,  U03A6
  "&#935; &Chi;" ,     # greek capital letter chi,  U03A7
  "&#936; &Psi;" ,     # greek capital letter psi,  U03A8
  "&#937; &Omega;" ,   # greek capital letter omega,  U03A9

  "&#945; &alpha;" ,   # greek small letter alpha, U03B1
  "&#946; &beta;" ,    # greek small letter beta,  U03B2
  "&#947; &gamma;" ,   # greek small letter gamma,  U03B3
  "&#948; &delta;" ,   # greek small letter delta,  U03B4
  "&#949; &epsilon;" , # greek small letter epsilon,  U03B5
  "&#950; &zeta;" ,    # greek small letter zeta,  U03B6
  "&#951; &eta;" ,     # greek small letter eta,  U03B7
  "&#952; &theta;" ,   # greek small letter theta,  U03B8
  "&#953; &iota;" ,    # greek small letter iota,  U03B9
  "&#954; &kappa;" ,   # greek small letter kappa,  U03BA
  "&#955; &lambda;" ,  # greek small letter lambda,  U03BB
  "&#956; &mu;" ,      # greek small letter mu,  U03BC
  "&#957; &nu;" ,      # greek small letter nu,  U03BD
  "&#958; &xi;" ,      # greek small letter xi,  U03BE
  "&#959; &omicron;" , # greek small letter omicron,  U03BF
  "&#960; &pi;" ,      # greek small letter pi,  U03C0
  "&#961; &rho;" ,     # greek small letter rho,  U03C1
  "&#962; &sigma;" ,   # greek small letter final sigma,  U03C2
  "&#963; &sigma;" ,   # greek small letter sigma,  U03C3
  "&#964; &tau;" ,     # greek small letter tau,  U03C4
  "&#965; &upsilon;" , # greek small letter upsilon,  U03C5
  "&#966; &phi;" ,     # greek small letter phi,  U03C6
  "&#967; &chi;" ,     # greek small letter chi,  U03C7
  "&#968; &psi;" ,     # greek small letter psi,  U03C8
  "&#969; &omega;" ,   # greek small letter omega,  U03C9
  "&#977; &theta;" ,   # greek small letter theta symbol,  U03D1
  "&#978; &upsilon;" , # greek upsilon with hook symbol,  U03D2
  "&#982; &pi;" ,      # greek pi symbol,  U03D6

  "&#8226; º" ,  # bullet, =black small circle, U2022 ISOpub
  "&#8230; º" ,  # horizontal ellipsis, =three dot leader, U2026 ISOpub
  "&#8242; '" ,  # prime, =minutes, =feet, U2032 ISOtech
  "&#8243; ''" , # double prime, =seconds, =inches, U2033 ISOtech
  "&#8254; -" ,  # overline, =spacing overscore, U203E
  "&#8260; /" ,  # fraction slash, U2044
  
  "&#16412; \"",   # opening double quote
  "&#16413; \"",   # closing double quote
  "&#16422; ..." ) # elipsis
}

# code from UseModWiki
sub WikiLinesToHtml {
  $IndentLimit = 6 ;
  my ($pageText) = @_;
  my ($pageHtml, @htmlStack, $code, $depth, $oldCode);

  chomp ($pageText) ;
  @htmlStack = ();
  $depth = 0;
  $pageHtml = "";
  $lines = 0 ;
  undef @td_depth ;
  foreach (split(/<br>/, $pageText)) {  # Process lines one-at-a-time
    $lines ++ ;
    $_ .= "<br>";

    # turn ''' .. ''' into <b> ... </b> etc
    $_ =~ s/('*)'''(.*?)'''/$1<b>$2<\/b>/g;
    $_ =~ s/''(.*?)''/<i>$1<\/i>/g;

    # turn == .. == into <h2> .. </h2> etc
    $_ =~ s/^[\s]*(\=\=+)([^\=]+)\=\=+/&WikiHeading($1, $2)/geo;

    if (s/^(\;+)([^:]+\:?)\:/<dt>$2<dd>/) {
      $code = "dl";
      $depth = length $1;
    } elsif (s/^(\:+)/<dt><dd>/) {
      $code = "dl";
      $depth = length $1;
    } elsif (s/^(\*+)/<li>/) {
      $code = "ul";
      $depth = length $1;
    } elsif (s/^(\#+)/<li>/) {
      $code = "ol";
      $depth = length $1;
    } elsif (($lines > 1) && (/^[ ]+[^\<]+/)) {
      # "    <table" should not qualify but it seems to do
      # again use workaround for lack of understanding of general expression
      $test = $_ ;
      $test =~ s /^[\s]+// ;
      if (substr ($test,0,1) ne "<")
      {
        $code = "pre";
        $depth = 1;
      }
    } else {
      $depth = 0;
    }
    while (@htmlStack > $depth) {   # Close tags as needed
      $pageHtml .=  "</" . pop(@htmlStack) . "><br>";
    }
    if ($depth > 0) {
      $depth = $IndentLimit  if ($depth > $IndentLimit);
      if (@htmlStack) {  # Non-empty stack
        $oldCode = pop(@htmlStack);
        if ($oldCode ne $code) {
          $pageHtml .= "</$oldCode><$code><br>";
        }
        push(@htmlStack, $code);
      }
      while (@htmlStack < $depth) {
        push(@htmlStack, $code);
        $pageHtml .= "<$code><br>";
      }
    }
    s/^\s*$/<p><br>/;                        # Blank lines become <p> tags
    $pageHtml .= $_;
#   $pageHtml .= &CommonMarkup($_, 1, 2);  # Line-oriented common markup
  }
  while (@htmlStack > 0) { # Clear stack
    $pageHtml .=  "</" . pop(@htmlStack) . "><br>";
  }
  return $pageHtml;
}

# modified from UseModWiki
sub WikiHeading {
  my ($depth, $text) = @_;

  $depth = length($depth);
  $depth = 6 if ($depth > 6);
  $text  =~ s/^\s+// ;
  $text  =~ s/\s+$// ;
  return "<h$depth>$text</h$depth>";
}

sub NoWiki
{
  my $text = shift ;
  $text =~ s/</&lt;/g ;
  $text =~ s/>/&gt;/g ;
  $text =~ s/\[/&#91;/g ;
  $text =~ s/\]/&#93;/g ;
  $text =~ s/\*/&#42;/g ;
  return ($text) ;
}

sub UnicodeToHtml
{
  my $text  = shift ;
  my $html = "" ;
  my $c, my $len, my $byte, my $ord, my $unicode, my $bytes, my $html ;

  $len = length ($text) ;
  for ($c = 0 ; $c < $len ; $c++)
  {
    $byte = substr ($text,$c,1) ;
    $ord = ord ($byte) ;
    if ($ord < 128)      # plain ascii character
    { $html .= $byte ; } # (will not occur in this script)
    else
    {
      # single byte left >= 0x80 ? should never occur but does a few times
      # treat as pre-unicode high ascii character
      if ($c == $len - 1)
      {
        $html = "\&\#". $ord . ";" ;
        push @errors, $title .":invalid unicode char ".$text. "\n" ;
      }
      else
      {
        if ($ord < 224)
        { $bytes = 2 ; }
        elsif ($ord < 240)
        { $bytes = 3 ; }
        elsif ($ord < 248)
        { $bytes = 4 ; }
        elsif ($ord < 252)
        { $bytes = 5 ; }
        else
        { $bytes = 6 ; }
        $unicode = substr ($text,$c,$bytes) ;
        $html .= &UnicodeToHtmlTag ($unicode) ;
        $c += $bytes - 1 ;
      }
    }
  }
  return ($html) ;
}


sub UnicodeToHtmlTag
{
  my $unicode = shift ;
  my $char = substr ($unicode,0,1) ;
  my $ord = ord ($char) ;
  my ($c, $value, $html) ;

  if ($ord < 128)         # plain ascii character
  { return ($unicode) ; } # (will not occur in this script)
  else
  {
    if ($ord >= 252)
    { $value = $ord - 252 ; }
    elsif ($ord >= 248)
    { $value = $ord - 248 ; }
    elsif ($ord >= 240)
    { $value = $ord - 240 ; }
    elsif ($ord >= 224)
    { $value = $ord - 224 ; }
    else
    { $value = $ord - 192 ; }
    for ($c = 1 ; $c < length ($unicode) ; $c++)
    { $value = $value * 64 + ord (substr ($unicode, $c,1)) - 128 ; }
    $html = "\&\#" . $value . ";" ;

    if ($testmode2)
    {
      # find html unicode tags that will remain after ascii substitution
      # these will cause problems on Palm/EPOC or in TomeRaider index in Pocket PC
      # substitions will have to be added
      if (($html eq &HtmlToAscii ($html)) &&
          ($html eq &HtmlToAsciiApproximation ($html)))
      { @unicodes {$unicode} ++ ; }
    }
    return ($html) ;
  }
}

sub HtmlToAscii
{
  my $html = shift ;
  my $value = $html ;
  my $ndx ;
  if ($html =~ /\&\#[\d]{2,5}\;/)
  {
    $value =~ s/\&\#([\d]{2,5})\;/$1/ ;
    if ($value < 256)
    { return (chr ($value)) ; }
    else
    { return ($html) ; }
  }
  else
  {
    $ndx = index ($asciicodes, $html) ;
    if ($ndx != -1)
    { return (substr ($asciicodes,$ndx-1,1)) ; }
    else
    { return ($html) ; }
  }
}

sub HtmlToAsciiApproximation
{
  my $html = shift ;
  my $ascii ;
  if ($html =~ /\&\#[\d]{2,5}\;/)
  {
    if ($language eq "eo") # try X-base replacement first
    {
      $ascii = @asciicodesEsperanto {$html} ;
      if (defined $ascii)
      { return ($ascii) ; }
    }
    
    $ascii = @asciicodes2 {$html} ;
    if (defined $ascii)
    { return ($ascii) ; }
    else
    { return ("×") ; }
  }
  else
  { return ("×") ; }
}

sub ShowProgress
{
  my $articles = shift ;
  if ($articles % 1000 == 0)
  {
    print $articles / 1000 . " " ;
    if ($articles % 15000 == 0)
    { print "\n" ; }
  }
}

sub TitleToIndex
{
  my $title = shift ;
  $letter = uc (substr ($title,0,1)) ;
  if ($letter lt "A")
  { $letter = "A" ; }
  if ($letter gt "Z")
  { $letter = "Z" ; }
  return (ord ($letter) - ord ("A")) ;
}

sub mmss
{
  $seconds = shift ;
  return (int ($seconds / 60) . " min, " . ($seconds % 60) . " sec") ;
}

sub GetDate
{
  my $time = shift ;
  my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime($time);

  my @weekdays_en = qw(Sunday Monday Tuesday Wednesday Thursday Friday Saturday);

  my @months_en   = qw (January February March April May June July
                        August September October November December);

  my @weekdays_nl = qw(zondag maandag dinsdag woensdag donderdag vrijdag zaterdag) ;

  my @months_nl   = qw (januari februari maart april mei juni juli
                        augustus september oktober november december) ;

  my @weekdays_de = qw (Sonntag Montag Dienstag Mittwoch Donnerstag Freitag Samstag);

  my @months_de   = qw (Januar Februar März April Mai Juni Juli
                        August September Oktober November Dezember);

  my @weekdays_fr = qw (dimanche, lundi, mardi, mercredi, jeudi, vendredi,samedi);

  my @months_fr   = qw (janvier février mars avril mai juin juillet
                        août septembre octobre novembre décembre );

  my @weekdays_eo = qw (dimancxo lundo mardo merkredo jxaudo vendredo sabato);

  my @months_eo   = qw (januaro februaro marto aprilo majo junio julio
                        auxgusto septembro oktobro novembro decembro);

  if ($language eq "nl")
  { return (@weekdays_nl[$wday] . " " . $mday . " " . @months_nl[$mon] . " " . (1900 + $year)) ; }
  elsif ($language eq "de")
  { return (@weekdays_de[$wday] . " " . $mday . " " . @months_de[$mon] . " " . (1900 + $year)) ; }
  elsif ($language eq "fr")
  { return (@weekdays_fr[$wday] . " " . $mday . " " . @months_fr[$mon] . " " . (1900 + $year)) ; }
  elsif ($language eq "eo")
  { return (@weekdays_eo[$wday] . " " . $mday . " " . @months_eo[$mon] . " " . (1900 + $year)) ; }
  else
  { return (@weekdays_en[$wday] . ", " . @months_en[$mon] . " " .  $mday . ", " . (1900 + $year)) ; }
}

# this routine is only used when demo database (small subset of articles) should be prepared
sub ScanLinks
{
  my $title_lc = lc ($title) ;
  #scan hyperlinks, add to @demo2 when a link refers to article in @demo
  if (index ($article ,'[[') != -1)
  {
    @segments = split ('\[\[', $article) ;
    for ($i = 1 ; $i <= $#segments ; $i++)
    {
      if (index (@segments [$i], ']]') != -1)
      {
        ($link, $remainder) = split (']]', @segments [$i], 2) ;
        $link =~ s/_/ /g ;

        if (index ($link, "|") != -1)
        { ($search, $show) = split ('\|', $link) ; }
        else
        { $search = $link ; }

        if (defined (@demo {lc($search)}))
        {
          @demo2 {$title_lc} ++ ;
        }
      }
    }
  }
}

sub x
{
  my $int = shift ;
  $code = sprintf ("%x,%x", int ($int / 256), $int % 256) ;
  return ($code) ;
}

#UTF-8 works as follows:
#ENCODING
#       The following byte sequences are used to represent a char­
#       acter. The sequence to be used depends  on  the  UCS  code
#       number of the character:
#       0x00000000 - 0x0000007F:
#           0xxxxxxx
#
#       0x00000080 - 0x000007FF:
#           110xxxxx 10xxxxxx
#
#       0x00000800 - 0x0000FFFF:
#           1110xxxx 10xxxxxx 10xxxxxx
#
#       0x00010000 - 0x001FFFFF:
#           11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
#
#       0x00200000 - 0x03FFFFFF:
#           111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
#
#       0x04000000 - 0x7FFFFFFF:
#           1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
#
#       The  xxx  bit  positions  are  filled with the bits of the
#       character code number in binary representation.  Only  the
#       shortest  possible  multibyte sequence which can represent
#       the code number of the character can be used.

