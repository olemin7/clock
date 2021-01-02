var config={
	bed:{
		min:[0,-5,0],
        size:[200,200,200],
		coner:15
	},
	feedrate:{
        max_xy:10000,
        max_z:1000,
		move_xy:4000,
		move_z:300
	},
    probe:{
        feedrate_xy:750,
        feedrate_z_base:20,
        grid:5,
        distance_min:0.1,
        distance_max:10,
        multiple:{
            delta:0.5
        },
        double:{
            hop:0.5,
            distance:-1
        },
        area:{
            distance:0.5
        }
    },
    engraver:{
        ToolChangeZ:20
    }
};

const SDcontrolMode_esp=1;
const SDcontrolMode_marlin=0;