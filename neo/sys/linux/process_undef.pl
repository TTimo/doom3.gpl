#!/usr/bin/env perl
# send linker output to stdin
# make 2>&1 | process_undef.pl

my %symbols;
open($list, "cat $ARGV[0] " . '| grep \'undefined reference\' | grep -v \'more undefined\' | sed -e \'s/.*undefined reference to `\(.*\)./\1/\' |');
my $line = <$list>;
do
{
  chop($line);
  $symbols{$line}++;
} until (!($line = <$list>));

@sorted = sort { $symbols{$b} <=> $symbols{$a} } keys %symbols;

foreach (@sorted)
{
  print "$symbols{$_} : $_\n";
};

