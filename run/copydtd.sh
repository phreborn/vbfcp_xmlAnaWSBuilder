ds=$(ls config)

for d in $ds;do

  cp AnaWSBuilder.dtd config/${d}/
  cp AnaWSBuilder.dtd config/${d}/channel/
  cp AnaWSBuilder.dtd config/${d}/model

done
