<!DOCTYPE HTML>
<html>
<head>
    <meta charset="utf-8">
    <title>img to hex</title>
</head>
<body>
<?php

if(!isset($_FILES['image'])) {
    ?>
    <form action="" enctype="multipart/form-data" method="post">
        <input type="file" name="image"/>
        <input type="submit" value="Send"/>
    </form>
<?php
}
else {
    $target_dir = __DIR__;
    $target_file = $target_dir . basename($_FILES["image"]["name"]);
    $uploadOk = 1;
    $imageFileType = pathinfo($target_file, PATHINFO_EXTENSION);
    if(isset($_POST["submit"])) {
        $check = getimagesize($_FILES["image"]["tmp_name"]);
        if($check === false) {
            die();
        }
    }

    if(move_uploaded_file($_FILES["image"]["tmp_name"], $target_file)) {

        if(exif_imagetype($target_file) != 1) {
            die();
        }

        $img = imageCreateFromGif($target_file);
        list($width, $height, $type, $attr) = getimagesize($target_file);

        $lines = floor($height / 8);

        for($li = 0; $li < $lines; $li++) {
            for($i = 0; $i < $width; $i++) {
                $tmp = '';
                for($j = 8; $j >= 1; $j--) {
                    $tmp .= (string)imagecolorat($img, $i, ($j - 1 + ($li * 8)));// > 127 ? 1 : 0;
                }
                echo '0x' . dechex(bindec($tmp)) . ', ';
            }
        }
    }
}
?>
</body>
</html>