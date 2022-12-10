# perl zz.pl <D3.1.text | recode -f mac..utf8

my @a;

while (<>) {
  chomp;

  if (/"(\d+), (\d+)\"/) {
     print "\r" if ($1 % 16 == 0);

     $a[$2] = $1;

    printf "0x%02x, ", $2;
  }
}

for my $c (0..$#a) {
  print "\r" if ($c % 32 == 0);

  if ($a[$c] >= 32 && $a[$c] < 255 && $a[$c] != 127) {
    print chr($a[$c]);
  } else {
    print ".";
  }
}

print "\r";
