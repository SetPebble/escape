<?php 

//  constants

define('SOURCE_IMAGE', 'room.png');
define('DEST_DIRECTORY', '../resources/data/');
define('WIDTH', 144);
define('HEIGHT', 168);

//  code

$img_src = @imagecreatefrompng(SOURCE_IMAGE);
if ($img_src) {
  //  get source image size
  $width_src = imagesx($img_src);
  $height_src = imagesy($img_src);
  //  loop through rows
  for ($row = 0;  $row < floor(($height_src + HEIGHT - 1) / HEIGHT);  $row++) {
    for ($column = 0;  $column < floor(($width_src + WIDTH - 1) / WIDTH);  $column++) {
      //  create destination image
      //$img_dest = imagecreatetruecolor(WIDTH, HEIGHT);
      $img_dest = @imagecreate(WIDTH, HEIGHT);
      if ($img_dest) {
        //  copy
        if (imagecopyresized($img_dest, $img_src, 0, 0, $column * WIDTH, $row * HEIGHT, WIDTH, HEIGHT, WIDTH, HEIGHT)) {
          //  get new filename
          $filename = 'room' . $column . $row . '.png';
          //  save image
          if (!imagepng($img_dest, DEST_DIRECTORY . $filename, 9))
            die('trouble saving file "' . $filename . '"');
          //  done with copy
          imagedestroy($img_dest);
        }
      }
    }
  }
  //  done with source
  imagedestroy($img_src);
}
?>