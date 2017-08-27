(cond
 ((string-equal system-type "windows-nt") ; Microsoft Windows
  (progn
	((c-mode .
			 ((company-clang-arguments . ("-IE:/Projects/symmerty/include" "-IE:\\Projects\\symmerty\\third_party\\windows\\SDL2-2.0.5\\include"))
			  (flycheck-clang-include-path  . ("E:/Projects/symmerty/include")))
			 ) 
	 ((c++-mode . ((mode . c))))
	 )
	))
 ((string-equal system-type "gnu/linux") ; linux
  (progn
	((c-mode .
			 ((company-clang-arguments . ("-I/mnt/Dev/Projects/symmetry/include"))
			  (flycheck-clang-include-path  . ("/mnt/Dev/Projects/symmetry/include")))
			 ) 
	 ((c++-mode . ((mode . c))))
	 )
	)))



