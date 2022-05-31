var express = require('express');

var app = express();
var Sunwell = require('../sunwell');
var sunwell = new Sunwell({ bodyFontSize: 35 });

app.get('/', function(req, res) {
    var width = 400;

    var card_name = "";
    var card_data = "";
    var fs = require('fs');

    var obj;
    fs.readFile(process.cwd() + '/examples/carddata', 'utf8', function (err, data) {
        if (err) throw err;
        obj = JSON.parse(data);

        var card = sunwell.createCard(obj, width, function(err, buffer) {
            if (err) next(err);

            res.writeHead(200, {
                //'Cache-Control': 'max-age=86400',
                'Content-Type': 'image/png'
            });

            res.write(buffer);
            res.end();

            fs.writeFile(process.cwd() + '/../hm_web/static/custom/' + obj['id'] + '.jpg', buffer, function(err) {
                if(err) {
                    return console.log(err);
                }

                console.log("The file " + obj['id'] + " was saved!");

            });
        });
    });
});

app.use(function(err, req, res, next) {
    next(err);
});

var listener = app.listen(process.env.PORT || 8081, function() {
    console.log('Server listening on port', listener.address().port);
});
