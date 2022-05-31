node-sunwell
============

[![Build Status](https://travis-ci.org/matkl/node-sunwell.svg?branch=master)](https://travis-ci.org/matkl/node-sunwell)
[![Coverage Status](https://coveralls.io/repos/github/matkl/node-sunwell/badge.svg?branch=master)](https://coveralls.io/github/matkl/node-sunwell?branch=master)
[![npm version](http://img.shields.io/npm/v/node-sunwell.svg?style=flat)](https://npmjs.org/package/node-sunwell "View this project on npm")


An HTML5 canvas based renderer for hearthstone cards for node.js. Forked from [HearthSim/sunwell](https://github.com/HearthSim/sunwell).

Requirements
------------

See [HearthSim/sunwell](https://github.com/HearthSim/sunwell) for detailed information. Card artworks and fonts are not included. Set `textureFolder` to the folder where you put the card artworks when creating the sunwell object.

Installation
------------
```
$ npm install --save node-sunwell
```

Usage
-----
Set up sunwell with your own settings:

```javascript
var Sunwell = require('node-sunwell');
var sunwell = new Sunwell({
	titleFont: 'Belwe',
	bodyFont: 'ITC Franklin Condensed',
	bodyFontSize: 24,
	bodyLineHeight: 55,
	bodyFontOffset: {x: 0, y: 0},
	textureFolder: __dirname + '/artworks/',
	smallTextureFolder: __dirname + '/smallArtworks/',
	debug: false
});
```	

###Rendering a card
To render a specific card, you can call the method `createCard()` of the `sunwell` object.

```javascript
var cardObj = sunwell.createCard(cardData, width, function(err, buffer) {
	// buffer contains PNG data
});
```

The `cardData` parameter is an object containing information about the card to be rendered. `width`
defines the width of the card to be rendered. Sunwell provides a "native" resolution up to 764x1100
pixels. While you can set a higher value than 764 for the desired render, it will only result in blurry
results. The max supported resolution of sunwell is already by far greater than in the game itself.

The callback will be invoked with the rendered card buffer.

The object you pass as `cardData` can be obtained for example through [HearthstoneJSON](https://hearthstonejson.com/).

```javascript
{
	"id":"CS2_087",
	"artist":"Zoltan Boros",
	"set":"CORE",
	"type":"SPELL",
	"rarity":"FREE",
	"cost":1,
	"name":"Blessing of Might",
	"flavor":"\"As in, you MIGHT want to get out of my way.\" - Toad Mackle, recently buffed.",
	"playRequirements":{"REQ_TARGET_TO_PLAY":0,"REQ_MINION_TARGET":0},
	"collectible":true,
	"playerClass":"PALADIN",
	"howToEarnGolden":"Unlocked at Level 45.",
	"howToEarn":"Unlocked at Level 1.",
	"text":"Give a minion +3 Attack.",
	"texture":"W16_a053_D"
}
```

Some properties are purely optional, since they are not used by sunwell, but these ones are required:

```javascript
{
	"id":"CS2_087",
	"set":"CORE",
	"type":"SPELL",
	"rarity":"FREE",
	"cost":1,
	"name":"Blessing of Might",
	"playerClass":"PALADIN",
	"text":"Give a minion +3 Attack.",
	"texture":"W16_a053_D"
}
```

In case of a minion or weapon card, you also need to pass `health` (or `durability`, but health is fine for weapons, too) and/or `attack`.

The method will return an object that provides an interface to manipulate the card after its creation.

If you want to update certain properties on the original `cardData`, simply call `cardObj.update()` and
pass an object with the properties you want to overwrite.

###Changing the number colors

If you want to make the numbers appear green/red, you can also pass in the following properties on your card object to both 
`createCard()` and/or `cardObj.update()`:

```javascript
{
	"costStyle": "0",
	"attackStyle": "+",
	"healthStyle": "-",
	"durabilityStyle": "-"
}
```

All the style default to "0". Setting the style to "+" makes the number appear green, setting it to "-" makes it appear red.


###Silence a minion
To silence a minion, set `silenced: true` either when creating the card, or with the update function.

###Let a card cost health instead of mana
Introduced by [Cho'gall](http://hearthstonelabs.com/cards#lang=enUS;detail=OG_121), cards may cost health instead of mana.
You can switch any cards cost icon by setting `costHealth: true` either when creating the card, or through the update function.

## Examples

See [examples/](examples/).

## Author

Original work by Christian Engel <hello@wearekiss.com>

Node.js port by Matthias Klein <matthias@klein.pw>

## License

Sunwell is licensed
[MIT](http://choosealicense.com/licenses/mit/). The full license text is
available in the `LICENSE` file in the respective implementations' folder.

# This is not a standalone project!

[hm_sunwell](https://github.com/farb3yonddriv3n/hm_sunwell) is part of [hearthmod software stack](https://github.com/hearthmod/hearthmod)
