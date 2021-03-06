/*
 #Фишка в моём шрифте - инверсированные цифры 0-9
 {0xC1, 0xAE, 0xB6, 0xBA, 0xC1} => (0x10) = 0x30 inverse 0
 {0xFF, 0xBD, 0x80, 0xBF, 0xFF} => (0x10) =0x31 inverse 1
 {0x8D, 0xB6, 0xB6, 0xB6, 0xB9} => (0x10) =0x32 inverse 2
 {0xDE, 0xBE, 0xB6, 0xB2, 0xCC} => (0x10) =0x33 inverse 3
 {0xE7, 0xEB, 0xED, 0x80, 0xEF} => (0x10) =0x34 inverse 4
 {0xD8, 0xBA, 0xBA, 0xBA, 0xC6} => (0x10) =0x35 inverse 5
 {0xC1, 0xB6, 0xB6, 0xB6, 0xCD} => (0x10) =0x36 inverse 6
 {0xBE, 0xDE, 0xEE, 0xF6, 0xF8} => (0x10) =0x37 inverse 7
 {0xC9, 0xB6, 0xB6, 0xB6, 0xC9} => (0x10) =0x38 inverse 8
 {0xD9, 0xB6, 0xB6, 0xB6, 0xC1} => (0x10) =0x39 inverse 9
 */

//Программа на JS для отображения HEX кодов русских символов (для дисплея)
var trans = [];
for (var i = 0x410; i <= 0x44F; i++) trans[i] = i - 0x350; // А-Яа-я
trans[0x401] = 0xA8; // Ё
trans[0x451] = 0xB8; // ё

to_win_1251 = function(str) {
    var ret = [];
    for (var i = 0; i < str.length; i++) {
        var n = str.charCodeAt(i);
        if (typeof trans[n] != 'undefined') n = trans[n];
        if (n <= 0xFF) {
            ret.push('lcd.write(0x'+n.toString(16)+');');
        }
    }
    console.log(ret.join(' '));
};

var rusU = ['А','Б','В','Г','Д','Е','Ё','Ж','З','И','Й','К','Л','М','Н','О','П','Р','С','Т','У','Ф','Х','Ц','Ч','Ш','Щ','Ъ','Ы','Ь','Э','Ю','Я'];
var rusL = ['а','б','в','г','д','е','ё','ж','з','и','й','к','л','м','н','о','п','р','с','т','у','ф','х','ц','ч','ш','щ','ъ','ы','ь','э','ю','я'];

for(var i in rusU) {
    console.log(rusU[i]);
    to_win_1251(rusU[i]);
}
for(var i in rusL) {
    console.log(rusL[i]);
    to_win_1251(rusL[i]);
}
//to_win_1251('Привет');