DIR=.publish

publish:
	# publish
	mkdir -p ~/git/clones/$(DIR)
	rm -rf $(DIR)
	mkdir $(DIR)
	cp -r * $(DIR)
	cd $(DIR) && rm -rf test test.sh *.pyc customMod static/images static/images sessions cache/cardsxml_p1 cache/cardsxml_p3
	tar cvvf web.tar $(DIR) && mv web.tar ~/git/clones/$(DIR)
