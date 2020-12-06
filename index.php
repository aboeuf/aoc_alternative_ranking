<!DOCTYPE html>
<html>
<title>AOC Alternative Ranking System</title>
<head>
  <link rel="stylesheet" href="styles.css">
</head>
<body>
  <h1>Advent Of Code : classement alternatif </h1>
  <p>
    Classement alternatif bas&eacute; sur le temps pass&eacute; entre la premi&egrave;re et la deuxi&egrave;me &eacute;toile.<br>
    Chaque jour le premier remporte N points, le deuxi&egrave;me N - 1 points, etc. (avec N le nombre de joueurs).<br>
    Aucun point tant que la deuxi&egrave;me &eacute;toile n'est pas obtenue.<br>
    Ce nombre de points est multipli&eacute; par un facteur qui augmente lin&eacute;airement avec le nombre de jours.<br>
    Il vaut 1.0 pour le premier jour et va jusqu'&agrave; 3.0 pour le jour 25.<br>
    En cas d'&eacute;galit&eacute; du score au g&eacute;n&eacute;ral, c'est la somme des d&eacute;lais qui d&eacute;partage.<br>
    Toutes les propositions de modifications sont les bienvenues.
  </p>
  <?php
    echo exec('aoc_alternative_ranking');
  ?>
</body>
</html>
