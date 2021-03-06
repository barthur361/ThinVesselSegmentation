void tree_from_critical_points( const ModelSet& models, Graph<Edge, cv::Vec3d>& tree )
{
    Graph<Edge, cv::Vec3d> graph;

    // compute the projection points add them to graph as nodes
    for( unsigned i=0; i<models.tildaP.size(); i++ )
    {
        const int& lineid1 = models.labelID[i];
        const Line3D* line1   = models.lines[lineid1];
        const cv::Vec3i& pos1 = models.tildaP[i];
        const Vec3d proj = line1->projection( pos1 );
        graph.add_node( proj );
    }

    // Build edges around a neighborhood system
    for( unsigned i=0; i<models.tildaP.size(); i++ )
    {
        for( int n=0; n<26; n++ )
        {
            const int& lineid1 = models.labelID[i];
            const Line3D* line1   = models.lines[lineid1];
            const cv::Vec3i& pos1 = models.tildaP[i];

            cv::Vec3i pos2;
            Neighbour26::getNeigbour( n, pos1, pos2 );
            if ( !models.labelID3d.isValid( pos2 ) ) continue;

            const int lineid2 = models.labelID3d.at( pos2 );
            if ( lineid2==-1 ) continue;

            const Line3D* line2   = models.lines[lineid2];

            const Vec3d proj1 = line1->projection( pos1 );
            const Vec3d proj2 = line2->projection( pos2 );
            const Vec3d direction = proj1 - proj2;
            const double dist = sqrt( direction.dot( direction ) );

            // TODO: line id is equivalent to point id in this case
            graph.add_edge( Edge(lineid1, lineid2, dist) );
        }
    }

    tree.reset( graph.get_nodes() );

    // computing min span tree from a neighborhood system
    DisjointSet djs( graph.num_nodes() );
    std::priority_queue<Edge> edges = graph.get_edges();
    while( !edges.empty() && tree.num_edges()<tree.num_nodes()-1 )
    {
        Edge e = edges.top();
        const int sid1 = djs.find( e.node1 );
        const int sid2 = djs.find( e.node2 );
        if( sid1 != sid2 )
        {
            tree.add_edge( e );
            djs.merge( sid1, sid2 );
        }
        edges.pop();
    }

    graph.clear_edges();


    // Get critical points (or critical points) in the volume
    const unsigned char ENDPOINT_YES = 255;
    const unsigned char ENDPOINT_NO  = 144;
    const unsigned char UN_DEFINED  = 0;   // undefined
    Data3D<unsigned char> endpoints_mask1( models.labelID3d.get_size(), UN_DEFINED );
    int x, y, z;
    for( z=1; z<endpoints_mask1.SZ()-1; z++ )
    {
        for ( y=1; y<endpoints_mask1.SY()-1; y++ )
        {
            for( x=1; x<endpoints_mask1.SX()-1; x++ )
            {
                // Background point
                if( models.labelID3d.at( x,y,z )==-1 ) continue;

                // Already computed
                if( endpoints_mask1.at(x,y,z)!=UN_DEFINED ) continue;

                // Breath-first search begin
                std::queue<Vec3i> myQueue;
                myQueue.push( Vec3i(x,y,z) );
                while( !myQueue.empty() )
                {
                    Vec3i pos = myQueue.front();
                    myQueue.pop();

                    // Initial guess the this point to be a endpoint
                    endpoints_mask1.at( pos ) = ENDPOINT_YES;

                    // Transverse the neighbor hood system
                    for( int i=0; i<26; i++ )
                    {
                        Vec3i off_pos;
                        Neighbour26::getNeigbour( i, pos, off_pos );

                        // Out of boundary
                        if( !models.labelID3d.isValid( off_pos ) ) continue;

                        // Background
                        if( models.labelID3d.at(off_pos)==-1 ) continue;

                        if( endpoints_mask1.at(off_pos)==UN_DEFINED )
                        {
                            myQueue.push( off_pos );
                            endpoints_mask1.at( off_pos ) = ENDPOINT_YES;
                            endpoints_mask1.at( pos )     = ENDPOINT_NO;
                        }
                        else if( endpoints_mask1.at(off_pos)==ENDPOINT_YES )
                        {
                            endpoints_mask1.at( pos ) = ENDPOINT_NO;
                        }
                    }
                }
            }
        }
    }

    Data3D<unsigned char> endpoints_mask2( models.labelID3d.get_size(), UN_DEFINED );
    for(z=models.labelID3d.SZ()-2; z>=1; z--)
    {
        for(y=models.labelID3d.SY()-2; y>=1; y--)
        {
            for(x=models.labelID3d.SX()-2; x>=1; x--)
            {
                // Background point
                if( models.labelID3d.at( x,y,z )==-1 ) continue;

                // Already computed
                if( endpoints_mask1.at(x,y,z)!=UN_DEFINED ) continue;

                // Breath-first search begin
                std::queue<Vec3i> myQueue;
                myQueue.push( Vec3i(x,y,z) );
                while( !myQueue.empty() )
                {
                    Vec3i pos = myQueue.front();
                    myQueue.pop();

                    // Initial guess the this point to be a endpoint
                    endpoints_mask1.at( pos ) = ENDPOINT_YES;

                    // Transverse the neighbor hood system
                    for( int i=0; i<26; i++ )
                    {
                        Vec3i off_pos;
                        Neighbour26::getNeigbour( i, pos, off_pos );

                        // Out of boundary
                        if( !models.labelID3d.isValid( off_pos ) ) continue;

                        // Background
                        if( models.labelID3d.at(off_pos)==-1 ) continue;

                        if( endpoints_mask1.at(off_pos)==UN_DEFINED )
                        {
                            myQueue.push( off_pos );
                            GLViwerModel vis;
                            endpoints_mask1.at( off_pos ) = ENDPOINT_YES;
                            endpoints_mask1.at( pos )     = ENDPOINT_NO;
                        }
                        else if( endpoints_mask1.at(off_pos)==ENDPOINT_YES )
                        {
                            endpoints_mask1.at( pos ) = ENDPOINT_NO;
                        }
                    }
                }
            }
        }
    }

    // Endpoints (or Critical points) are the points that have only one neighbor
    vector<Vec3i> endpoints;
    for(z=1; z<models.labelID3d.SZ()-1; z++)
    {
        for (y=1; y<models.labelID3d.SY()-1; y++)
        {
            for(x=1; x<models.labelID3d.SX()-1; x++)
            {
                if( endpoints_mask1.at(x,y,z)==ENDPOINT_YES || endpoints_mask2.at(x,y,z)==ENDPOINT_YES )
                {
                    endpoints.push_back( Vec3i(x,y,z) );
                }
            }
        }
    }

    struct Dis_Pos
    {

        float dist;
        Vec3i to_pos;
        Dis_Pos( const float& distance, const Vec3i& position )
            : dist(distance) , to_pos(position) { }
        inline bool operator<( const Dis_Pos& right ) const
        {
            // reverse the sign of comparison for the use of priority_queue
            return ( dist > right.dist );
        }
    };

    const unsigned char VISITED_YES = 255;
    const unsigned char VISITED_N0  = 0;
    Data3D<unsigned char> isVisited( models.labelID3d.get_size(), VISITED_N0 );
    for( vector<Vec3i>::iterator it=endpoints.begin(); it<endpoints.end(); it++ )
    {
        const Vec3i& from_pos = *it;

        std::priority_queue< Dis_Pos > myQueue;
        myQueue.push( Dis_Pos( 0.0f, from_pos) );

        // Breath first search begin
        isVisited.reset( ); // set all the data to 0.
        isVisited.at( from_pos ) = VISITED_YES;

        bool to_pos_found = false;
        while( !myQueue.empty() && !to_pos_found )
        {
            Dis_Pos dis_pos = myQueue.top();
            myQueue.pop();

            // Search among neighborhood system
            for( int i=0; i<26; i++ )
            {
                // Get the propagate position
                Vec3i to_pos;
                Neighbour26::getNeigbour( i, dis_pos.to_pos, to_pos );

                if( !isVisited.isValid(to_pos) ) continue;
                if(  isVisited.at(to_pos)==VISITED_YES ) continue;

                // TODO: Assume that each point has its own line
                // You may need to change this in the future
                const int& lineid_from = models.labelID3d.at( from_pos );
                const int& lineid_to   = models.labelID3d.at( to_pos );

                const int& nodeid_from = lineid_from;
                const int& nodeid_to   = lineid_to;

                // If nodeid_to is a background point or
                if( nodeid_to==-1 || djs.find( nodeid_from )==djs.find( nodeid_to ) )
                {
                    Vec3i dif = to_pos - from_pos;
                    float dist = sqrt( 1.0f*dif.dot( dif ) );

                    // Prevent from searching too far aways
                    if( dist > 7.0f ) continue;

                    myQueue.push( Dis_Pos(dist, to_pos) );
                    isVisited.at( to_pos ) = VISITED_YES;
                }
                else
                {
                    // If these two points belong to different branches
                    const Line3D* line_from = models.lines[lineid_from];
                    const Line3D* line_to = models.lines[lineid_to];
                    const Vec3d proj_from = line_from->projection( from_pos );
                    const Vec3d proj_to =   line_to->projection( to_pos );

                    const Vec3d dir = proj_from - proj_to;
                    float dist = sqrt( 1.0f*dir.dot( dir ) );

                    graph.add_edge( Edge( nodeid_from, nodeid_to, dist) );

                    to_pos_found = true;
                    break;
                }
            }
        }
    }

    edges = graph.get_edges();
    while( !edges.empty() && tree.num_edges()<tree.num_nodes()-1 )
    {
        Edge e = edges.top();
        const int sid1 = djs.find( e.node1 );
        const int sid2 = djs.find( e.node2 );
        if( sid1 != sid2 )
        {
            tree.add_edge( e );
            djs.merge( sid1, sid2 );
        }
        edges.pop();
    }


}
