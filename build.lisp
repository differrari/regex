(load "~/redbuild/v3/redbuild.lisp")

(quick_redb (make-instance `redmod
        :name "regex"
        :type :lib
        :target (native)
        :srcs (list "regex.c")
) :add-dependencies t :run t :success (lambda () (print "Done") (print (emit_compile_commands))))