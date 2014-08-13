function [] = render_fp(floorplan, color_by_room, c, labels)
	% render_fp(floorplan)
	%
	%	Renders this floor plan on the current figure
	%
	% arguments:
	%
	%	floorplan -	The struct to render
	%
	%	color_by_room -	OPTIONAL. If true, will color each room
	%			separately. Default is false.
	%
	%	c -		OPTIONAL. Default color to use. Can specify
	%			[r g b] where each component is in [0,1],
	%			or can also specify optional [r g b alpha]
	%	
	%	labels -	OPTIONAL.  If true, will write room #'s.
	%			Default is false.
	%

	hold all;
	set(gcf, 'renderer', 'opengl');
	axis equal;
	axis off;

	% check arguments
	if(~exist('color_by_room', 'var'))
		color_by_room = false;
	end
	if(~exist('c', 'var'))
		c = [0.8 0.8 1 1];
	end
	if(length(c) < 4)
		c = [c, ones(1,4-length(c))];
	end
	if(~exist('labels', 'var'))
		labels = false;
	end

	% make a color for each room
	if(color_by_room)
		colors = 0.25 + 0.5*rand(floorplan.num_rooms, 3);
	else
		colors = ones(floorplan.num_rooms, 1) * c(1:3);
	end

	% plot triangles
	if(c(4) ~= 0)
		for i = 1:floorplan.num_tris
			fill(floorplan.verts(floorplan.tris(i,:),1), ...
				floorplan.verts(floorplan.tris(i,:),2), ...
				colors(floorplan.room_inds(i),:), ...
				'FaceAlpha', c(4), ...
				'EdgeColor', 'none');
		end
	end

	% print room indices
	if(labels)
		for i = 1:floorplan.num_rooms
			ts = find(floorplan.room_inds == i);
			vs = floorplan.tris(ts,:);
			x_avg = mean(floorplan.verts(vs(:),1));
			y_avg = mean(floorplan.verts(vs(:),2));
			name = num2str(i-1);
			text(x_avg, y_avg, name);
		end
	end

	% plot edges
	line([floorplan.verts(floorplan.edges(:,1),1)' ; ...
		floorplan.verts(floorplan.edges(:,2),1)'], ...
		[floorplan.verts(floorplan.edges(:,1),2)' ; ...
		floorplan.verts(floorplan.edges(:,2),2)'], ...
		'LineWidth', 2, 'color', 'black');
end
