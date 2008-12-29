-- test - find
testf = function( num, str, pat, res )
  local s, e, r
  s, e = str:find( pat )
  io.write( "[find "..num.."]\n" )
  io.write( "string = \""..str.."\"\n" )
  io.write( "pattern = \""..pat.."\"\n" )
  if( s ~= nil ) then
    r = str:sub( s, e )
    io.write( "result = \""..r.."\"\n" )
  else
    r = nil
    io.write( "result = nil\n" )
  end
  if( r == res ) then
    io.write( "> OK\n" )
  else
    io.write( "> NG\n" )
  end
end
-- find 1
str = "\\5à¢!üEƒF}\\}_ŸB5\"ú€x‚³‚º\"ƒF}_à¢‚³ŸBüExú€‚º!}"
pat = "ú€[^ŸB]*"
testf( 1, str, pat, "ú€x‚³‚º\"ƒF}_à¢‚³" )
-- find 2
str = "-ú@B@Yİ3ú~ZYƒIüƒAƒA@Ÿ~ú@ú~-%%Bü‚ºŸ~‚ºƒI3İZ"
pat = "%w+%W+"
testf( 2, str, pat, "B@" )
-- find 3
str = "]Aàüà~ }üD€üIy<ßüDƒF<]ƒF à~àİàüßüIàİ€y¢A}¢"
pat = "..€.."
testf( 3, str, pat, "}üD€üIy" )
-- find 4
str = "4ƒD.aaƒFƒF[Xàß.^Xü^ú€[*C@ƒH@ƒDƒHü4*Càßú€"
pat = "([ƒ@-ƒ–]+).*%1"
testf( 4, str, pat, "ƒD.aaƒFƒF[Xàß.^Xü^ú€[*C@ƒH@ƒD" )
-- find 5
str = "ßXƒGßüGBBƒG#*__0Ÿ~ŸAàü*|ƒDàüƒD#|Ÿ~0ŸAXüGƒIƒI"
pat = ".*(.).+%1"
testf( 5, str, pat, "ßXƒGßüGBBƒG#*__0Ÿ~ŸAàü*|ƒDàüƒD" )
-- find 6
str = "úüüG/ú€./#|£üA‚¼^#à£‚¼úü£üGz.à£üE!^ú€züEüA!|"
pat = "%l%L"
testf( 6, str, pat, "z." )
-- find 7
str = "cƒD@àÜ{c|za|{‚´àü{àBzàü@ƒDàÜ\{àB\‚´Y‚¹aY‚¹"
pat = "[@Y\\]"
testf( 7, str, pat, "Y" )
-- find 8
str = "úBab} ü@Z ]‚´à ƒE'}ƒEX\'Xb\úBúAaü@]ZúA‚´à "
pat = "%U+[ú@-úC]+"
testf( 8, str, pat, "b\úBúA" )
-- find 9
str = "ŸAƒI/}üA,.üAƒIƒEà@ƒEà@à~ƒA/<İà~úC‚»úCƒA‚»ŸA}.<İ,"
pat = "}[ü@üAüBüCüDüEüFüGüHüIüJüK]?[^Ÿ@ŸAŸBŸ~Ÿ€Ÿü]*ƒA"
testf( 9, str, pat, "}üA,.üAƒIƒEà@ƒEà@à~ƒA/<İà~úC‚»úCƒA" )
-- find 10
str = "ƒC5.‚¸ƒDƒCÜ.üHÜú€^‚³~~5=~‚³ƒD^(=Ş~üHú€‚¸(Ş"
pat = "%f[~].%d.."
testf( 10, str, pat, nil )
-- find 11
str = "`Ü`0|{úA{Ü)~úA@|~ƒF0;ƒF}3)3Ÿ€@Ÿ€üJüJ;}"
pat = "%d%D*%d"
testf( 11, str, pat, "0|{úA{Ü)~úA@|~ƒF0" )
-- find 12
str = "ú~|ƒ@|5‚¼ƒ@ü¡ú~àÜü0cc_à€0üKàÜ_‚¼üK\\}}à€\\¡5"
pat = "%d%S-%d"
testf( 12, str, pat, "5‚¼ƒ@ü¡ú~àÜü0" )
-- find 13
str = "+/úC~4~)(5ú€(*Ÿ€ú€üKà *üFŸ€à üKƒH5+)4(üF/ƒHúC"
pat = "%b()"
testf( 13, str, pat, "(*Ÿ€ú€üKà *üFŸ€à üKƒH5+)" )
-- find 14
str = "bßƒ‚¶ß‚»}B~}b`.à~@ƒ.~‚´„„+‚´+‚»ƒB„`‚¶"
pat = "%bƒ„[^%d%l%w%s]"
testf( 14, str, pat, "ƒ‚¶ß‚»}B~}b`.à~@ƒ.~‚´„„+" )
-- find 15
str = "üG‚»‚´B}‚¼8‚»ŸBü@8ü@‚¼.à¢üGàÜú€¢à~ŸBàÜBà~ú€}.à¢‚´¢"
pat = "%..-$"
testf( 15, str, pat, ".à¢üGàÜú€¢à~ŸBàÜBà~ú€}.à¢‚´¢" )
-- find 16
str = "à£ú@!!;{Şà£^)Z;Ÿüü@xZüG^ŞŸüx1{ü@üG){{1ú@"
pat = "^%a%A"
testf( 16, str, pat, nil )
-- find 17
str = "[à ¡üA/#[6/‚¶\,#=,{XŞà üFX¡6{‚¶üF\=üAŞ"
pat = "\\"
testf( 17, str, pat, nil )
-- find 18
str = "}ú€#\\5B}‚³;XXÜ\\B‚³AŸBüK5[àİàİ;ÜŸBú€[A#üK"
pat = "\\.-\\"
testf( 18, str, pat, "\\5B}‚³;XXÜ\\" )
-- find 19
str = "‚»à¢=€~|<à@üEà¢ ß~{ßƒC‚»Ÿ@ |_ƒCŸ@€{üE_=<à@"
pat = "^..............................$"
testf( 19, str, pat, "‚»à¢=€~|<à@üEà¢ ß~{ßƒC‚»Ÿ@ |_ƒCŸ@€{üE_=<à@" )
-- find 20
str = "|üAàÜ2‚´#_]ƒF#àÜŸB|àİBüG2àİ''‚´à _üG]à ŸBüABƒF"
pat = "üG[^a-eA-E%%]-[a-eA-E%%]?à "
testf( 20, str, pat, "üG2àİ''‚´à " )
-- test - gmatch
testgf = function( num, str, pat, res )
  local p, r
  r = ""
  io.write( "[gmatch "..num.."]\n" )
  io.write( "string = \""..str.."\"\n" )
  io.write( "pattern = \""..pat.."\"\n" )
  io.write( "result = \"" )
  for p in str:gmatch( pat ) do
    if( r == "" ) then
      r = r..p
    else
      r = r..","..p
    end
  end
  io.write( r.."\"\n" )
  if( r == res ) then
    io.write( "> OK\n" )
  else
    io.write( "> NG\n" )
  end
end
-- gmatch 1
str = "Bà {ƒB‚¸‚¸XƒIB)^'ƒH{àA'‚¼ƒHàAyà )/‚¼ƒByƒIX^/"
pat = ".[{/^']"
testgf( 1, str, pat, "à {,)^,ƒH{,àA',)/,X^" )
-- gmatch 2
str = "¡ƒIú~Ÿ€2;Ÿü=üC_¡ƒIŸ€|üF!ƒDú~=2‚³‚³_|ƒDŸü!üFüC;"
pat = "..[%d;=!]?"
testgf( 2, str, pat, "¡ƒI,ú~Ÿ€2,;Ÿü=,üC_,¡ƒI,Ÿ€|,üF!,ƒDú~=,2‚³,‚³_,|ƒD,Ÿü!,üFüC;" )
-- gmatch 3
str = "‚´!_[,,ƒGƒE5àü?5=?@ƒGc‚´ƒDƒD=!àüƒEYcY@[_"
pat = "[^,?]+"
testgf( 3, str, pat, "‚´!_[,ƒGƒE5àü,5=,@ƒGc‚´ƒDƒD=!àüƒEYcY@[_" )
-- gmatch 4
str = "*‚·ŸA¡+àü~üH~üHüA¡^ú€ú€ƒBàİŸAƒBàİŸA‚·^Şà£+à£àüŞ*üA"
pat = "(.+)%1"
testgf( 4, str, pat, "~üH,ú€,ƒBàİŸA" )
-- gmatch 5
str = ";$İ‚µ$‚»{Ÿ@.%¢à@üK{‚¹ú@.‚»‚µà@ú@¢zz;Ÿ@%İüK‚¹"
pat = ".((.).-)%2"
testgf( 5, str, pat, "$İ‚µ,{Ÿ@.%¢à@üK,ú@.‚»‚µà@,z" )
-- gmatch 6
str = "‚¹55@'BÜ\"/Ÿ~üAüIàİŸ~Ü@7‚¹:BúA7\"':üI/àİúAüA"
pat = "%W+"
testgf( 6, str, pat, "‚¹,@',Ü\"/Ÿ~üAüIàİŸ~Ü@,‚¹:,úA,\"':üI/àİúAüA" )
-- gmatch 7
str = "?àB?@üCúü`[CÜ@C[6üE`üCúüàBÜ‚¼<Ÿ€üE6,Ÿ€‚¼,<"
pat = "[^%z\001-\255]+"
testgf( 7, str, pat, "àB,üCúü,üE,üCúüàB,‚¼,Ÿ€üE,Ÿ€‚¼" )
-- gmatch 8
str = "üEÜàÜ]^Ü~\"#àÜYƒB#}\"üGYŞ^]8}CüG8üE~ƒBŞC"
pat = "[%z\001-\255]+"
testgf( 8, str, pat, "Ü,]^Ü,\"#,Y,#,\",YŞ^]8,C,8,ŞC" )
-- gmatch 9
str = ".‚¼x>ZƒDzßàŞ8zßZ/àŞ[ƒD/8‚¼@).)@x>[[["
pat = "%w%W%W-%w"
testgf( 9, str, pat, "x>Z,zßàŞ8,zßZ,8‚¼@).)@x" )
-- gmatch 10
str = "YbƒFüH_}ƒE8b8aüHƒFüIY‚´àßƒE‚´_aİüI'@'àß}İ@"
pat = "%f[üIàßüH_}].[^üIàßüH_}]+"
testgf( 10, str, pat, "üHƒF,üIY‚´,àßƒE‚´,_aİ,üI'@'" )
-- test - gsub
testgs = function( num, str, pat, repl, res )
  local r
  r = ""
  io.write( "[gsub "..num.."]\n" )
  io.write( "string = \""..str.."\"\n" )
  io.write( "pattern = \""..pat.."\"\n" )
  io.write( "rstring = \""..tostring( repl ).."\"\n" )
  io.write( "result = \"" )
  r = str:gsub( pat, repl )
  io.write( r.."\"\n" )
  if( r == res ) then
    io.write( "> OK\n" )
  else
    io.write( "> NG\n" )
  end
end
-- gsub 1
testgs( 1, "‚ ‚¢‚¤‚¦‚¨‚¨‚¦‚¤‚¢‚ ", "([‚¢‚¤])([‚¢‚¤])", "%2 %1", "‚ ‚¤ ‚¢‚¦‚¨‚¨‚¦‚¢ ‚¤‚ " )
-- gsub 2
testgs( 2, "CBAŸ~ABCŸ€Ÿü@@@@@[AAA\BB]C^C_BB{AAA|@@@@}~Ÿ@ACŸACAŸBBA", "[@ABC]+", "", "Ÿ~Ÿ€Ÿü@[\]^_{|}~Ÿ@ŸAŸB" )
-- gsub 3
repl = {}
repl["‚ "] = "ƒA"; repl["‚¢"] = "ƒC"; repl["‚¤"] = "ƒE"; repl["‚¦"] = "ƒE"; repl["‚¨"] = "ƒI";
testgs( 3, "‚©‚ ‚«‚¢‚¢‚¢‚­‚¤‚¯‚¨‚¤", ".", repl, "‚©ƒA‚«ƒCƒCƒC‚­ƒE‚¯ƒIƒE" )
-- gsub 4
repl = function( word1, word2 )
  local r
  r = ""
  if( word1 ~= "" ) then
    r = "["..word1.."]"
  end
  if( word2 ~= "" ) then
    r = r.."["..word2.."]"
  end
  return r
end
testgs( 4, "‚à‚± ƒ‚ƒR ‚Ó‚É ƒtƒj ‚Óƒ‚ƒt‚à", "([‚Ÿ-‚ñ]*)%S?(%S?)", repl, "[‚à‚±] [ƒR] [‚Ó‚É] [ƒj] [‚Ó][ƒt][‚à]" )
-- test - format
testfm = function( num, frm, str, res )
  local r
  io.write( "[format "..num.."]\n" )
  io.write( "format = '"..frm.."'\n" )
  io.write( "string = '"..str.."'\n" )
  io.write( "result = '" )
  r = string.format( frm, str )
  -- r = string.format('%q', 'a string with "quotes" and \n new line')
  io.write( r.."'\n" )
  if( r == res ) then
    io.write( "> OK\n" )
  else
    io.write( "> NG\n" )
  end
end
-- format 1
testfm( 1, '%q', '\]\\\\t\\]%%$"˜¥"DON', '"\]\\\\\\t\\\\]%%$\\"˜¥\\"DON"' )
-- format 2
testfm( 2, 'te%qst', '\000', 'te"\\000"st' )
-- test - match
testm = function( num, str, pat, res1, res2 )
  local r1, r2
  r1, r2 = str:match( pat )
  io.write( "[match "..num.."]\n" )
  io.write( "string = \""..str.."\"\n" )
  io.write( "pattern = \""..pat.."\"\n" )
  io.write( "result1 = \""..tostring( r1 ).."\"\n" )
  io.write( "result2 = \""..tostring( r2 ).."\"\n" )
  if( r1 == res1 and r2 == res2 ) then
    io.write( "> OK\n" )
  else
    io.write( "> NG\n" )
  end
end
-- match 1
testm( 1, "[[http://google.com]]", "(https?)://([%w./]*)", "http", "google.com" )
-- match 2
testm( 2, "’Lq’Lq‚½‚Á‚Õ‚è‚½‚ç‚±", "(‚½?)[^‚ç]+(‚ç?)", "", "‚ç" )
-- match 3
testm( 3, string.sub( "ABC‚³‚µ‚·EFG›~€", 1, 17 ), "[^\129-\159\224-\252%z]+", "ABC‚³‚µ‚·EFG›~", nil )
-- match 4
testm( 4, "AAAàAàAA‚ AABC‚ A‚¢A‚ ABàAA‚ AAA‚ A‚¢A‚ CDAAàA", "(.*)(%w%w.*)%w%w", "AAAàAàAA‚ AABC‚ A‚¢A‚ ", "ABàAA‚ AAA‚ A‚¢A‚ " )
