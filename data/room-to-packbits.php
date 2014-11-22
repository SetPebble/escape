<?php 

//  constants

define('SOURCE_IMAGE', 'room.png');
define('DEST_DIRECTORY', '../resources/data/');
define('WIDTH', 96);
define('HEIGHT', 96);

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
      $img_dest = imagecreatetruecolor(WIDTH, HEIGHT);
      //$img_dest = @imagecreate(WIDTH, HEIGHT);
      if ($img_dest) {
        //  copy
        if (imagecopyresized($img_dest, $img_src, 0, 0, $column * WIDTH, $row * HEIGHT, WIDTH, HEIGHT, WIDTH, HEIGHT)) {
          //  initialize variables
          $snippet = null;
          $mode = 'init';
          //  start with width and height
          $data = pack('CC', WIDTH, HEIGHT);
          //  loop through image
          for ($y = 0;  $y < HEIGHT;  $y++) {
            for ($x = 0;  $x < WIDTH;  $x += 8) {
              //  calculate byte
              $byte = 0;
              for ($bit = 0;  $bit < 8;  $bit++)
                if ((imagecolorat($img_dest, $x + $bit, $y) & 0xff) > 0x80)
                  $byte |= (1 << $bit);
              $byte = pack('C', $byte);
              //  check mode
              switch ($mode) {
                case 'init':
                  $snippet = $byte;
                  $mode = 'unknown';
                  break;
                case 'unknown':
                  //  set to repeat or literal mode
                  $mode = ($byte == substr($snippet, -1)) ? 'repeat' : 'literal';
                  //  add to new repeat sequence
                  $snippet .= $byte;
                  break;
                case 'repeat':
                  if ($byte == substr($snippet, -1)) {
                    //  check for too much data
                    if (strlen($snippet) >= 0x80) {
                      //  add key for 128 repeated items
                      $data .= pack('c', 1 - strlen($snippet)) . substr($snippet, -1);
                      //  reset
                      $snippet = null;
                      $mode = 'unknown';
                    }
                  } else {
                    //  add repeated data so far
                    $data .= pack('c', 1 - strlen($snippet)) . substr($snippet, -1);
                    //  start new sequence
                    $snippet = null;
                    $mode = 'unknown';
                  }
                  //  add this value
                  $snippet .= $byte;
                  break;
                case 'literal':
                  if (($byte == substr($snippet, -1)) && (strlen($snippet) > 2)) {
                    //  save literals (except last character)
                    $data .= pack('C', strlen($snippet) - 2) . substr($snippet, 0, strlen($snippet) - 1);
                    //  restart repeat sequence
                    $snippet = $byte;
                    $mode = 'repeat';
                    //  check for too much data
                  } elseif (strlen($snippet) >= 0x80) {
                    //  add key for 128 items
                    $data .= pack('C', 0x7f) . $byte;
                    //  reset
                    $snippet = null;
                    $mode = 'unknown';
                  }
                  //  add this character
                  $snippet .= $byte;
                  break;
              }
            }
          }
          //  cleanup
          switch ($mode) {
            case 'unknown':
              //  just send one character
              $data .= pack('C', 0) . $snippet;
              break;
            case 'repeat':
              //  add remaining repeated bytes
              $data .= pack('c', 1 - strlen($snippet)) . substr($snippet, -1);
              break;
            case 'literal':
              //  add literal remainder
              $data .= pack('C', strlen($snippet) - 1) . $snippet;
              break;
          }
          if (($row == 0) && ($column == 1)) {
            //  debug
            print('data:');
            for ($i = 0;  $i < 20;  $i++)
              printf(' %02x', ord(substr($data, $i, 1)));
            echo "\n\n";
            //  test
            $pixels = array();
            $x = $y = 0;
            for ($ptr = $data;  strlen($ptr);  ) {
              $count = ord(substr($ptr, 0, 1));
              if ($count >= 128) {
                //  repeat sequence
                $count = 257 - $count;    //  2's complement conversion
                for ($i = 0;  $i < $count;  $i++) {
                  if (!isset($pixels[$y]))
                    $pixels[$y] = array();
                  $pixels[$y][] = substr($ptr, 1, 1);
                  if (++$x >= WIDTH / 8) {
                    $x = 0;
                    $y++;
                    if ($y >= HEIGHT)
                      break;
                  }
                }
                $ptr = substr($ptr, 2);
              } else {
                //  literal
                $count++;
                for ($i = 0;  $i < $count;  $i++) {
                  if (!isset($pixels[$y]))
                    $pixels[$y] = array();
                  $pixels[$y][] = substr($ptr, $i + 1, 1);
                  if (++$x >= WIDTH / 8) {
                    $x = 0;
                    $y++;
                    if ($y >= HEIGHT)
                      break;
                  }
                }
                $ptr = substr($ptr, 1 + $count);
              }
            }
            for ($y = 0;  $y < 30;  $y++) {
              foreach ($pixels[$y] as $x) {
                $byte = ord($x);
                for ($bit = 0;  $bit < 8;  $bit++) {
                  echo ($byte & (1 << $bit)) ? '.' : 'X';
                }
              }
              echo "\n";
            }
          }
          //  get new filename
          $filename = 'room_' . $column . '_' . $row . '.pbt';
          //  save packbits image
          file_put_contents('../resources/data/' . $filename, $data);
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
