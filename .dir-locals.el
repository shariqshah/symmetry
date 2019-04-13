(cond
 ((string-equal system-type "windows-nt") ; Microsoft Windows
  (progn
	((c-mode .
			 ((company-clang-arguments . ("-IW:/include"))
			  (flycheck-clang-include-path  . ("W:/include")))
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



