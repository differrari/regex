(load "~/redbuild/v3/redbuild.lisp")

; (redb-set-tester "tester.c")

(quick_redb (make-instance `redmod
        :name "regex"
        :type (dynlib)
        :target (native)
        :srcs (dynsrc "regex.c")
) :add-dependencies t :run t :success (lambda () (print "Done") (print (emit_compile_commands))))