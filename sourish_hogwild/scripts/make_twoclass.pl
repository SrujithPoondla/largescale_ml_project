while(<STDIN>) {
    my ($row, $col, $rating) = split(' ');
    if($col == 0) {
	$v = 2*($rating > 4) - 1;
	print "$row $col $v\n";
    } else {
	print;
    }
}
