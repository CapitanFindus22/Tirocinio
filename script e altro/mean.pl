#!/usr/bin/perl

use strict;
use warnings;
use feature 'say';
use POSIX qw(round);

my $filename = "file.txt";

open my $fh, '<', $filename or die "Could not open '$filename': $!";

my @numbers = grep { /^\d+$/ } <$fh>;

close $fh;

if (@numbers) {
    my $sum = 0;
    $sum += $_ for @numbers;
    say "Average: ", round($sum / @numbers);
}