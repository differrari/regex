(load "~/redbuild/v3/redbuild.lisp")

; (redbuild:set-tester "tester.c")

(redbuild:quick-build (redbuild:make-instance `redbuild:redmod
        :name "regex"
        :type (redbuild:dynlib)
        :target (redbuild:native)
        :srcs (redbuild:dynsrc "regex.c")
) :add-dependencies t :run t :success (lambda () (print "Done") (print (redbuild:emit-compile-commands))))