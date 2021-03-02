import {
  Model,
  DataTypes,
  HasManyAddAssociationMixin,
  Association,
  Optional,
  HasManyGetAssociationsMixin,
  HasOneSetAssociationMixin,
  HasOneGetAssociationMixin,
} from 'sequelize';

import sequelize from '../util/mysqlDatabase';
import User from './User';
import City from './City';
import Ride from './Ride';

interface GroupAttributes {
  // core attributes
  id: number;
  name: string;
  currentOccupancy: number;
  hidden: boolean;
  onRequest: boolean;
  bearing: number;
  // foreign keys
  leaderUserId: number;
  homeId: number;
  destinationId: number;
  // properties through associations
  members?: User[];
  leaderUser?: User;
  home?: City;
  destination?: City;
  rides?: Ride[];
}

interface GroupCreationAttributes extends Optional<GroupAttributes, 'id'> {}

export default class Group extends Model<GroupAttributes, GroupCreationAttributes> implements GroupAttributes {
  // core attributes
  public id!: number;
  public name!: string;
  public currentOccupancy!: number;
  public hidden!: boolean;
  public onRequest!: boolean;
  public bearing!: number;
  // foreign keys
  public leaderUserId!: number;
  public homeId!: number;
  public destinationId!: number;
  // timestamps
  public readonly createdAt!: Date;
  public readonly updatedAt!: Date;
  // getter setter methods
  public addMembers!: HasManyAddAssociationMixin<User[], number>;
  public getMembers!: HasManyGetAssociationsMixin<User[]>;
  public setLeaderUser!: HasOneSetAssociationMixin<User, number>;
  public getLeaderUser!: HasOneGetAssociationMixin<User>;
  public setHome!: HasOneSetAssociationMixin<City, number>;
  public getHome!: HasOneGetAssociationMixin<City>;
  public setDestination!: HasOneSetAssociationMixin<City, number>;
  public getDestination!: HasOneGetAssociationMixin<City>;
  public addRides!: HasManyAddAssociationMixin<Ride[], number>;
  public getRides!: HasManyGetAssociationsMixin<Ride[]>;
  // properties through associations
  public readonly members?: User[];
  public readonly leaderUser?: User;
  public readonly home?: City;
  public readonly destination?: City;
  public readonly rides?: Ride[];
  public static associations: {
    members: Association<Group, User>;
    leaderUser: Association<Group, User>;
    home: Association<Group, City>;
    destination: Association<Group, City>;
    rides: Association<Group, Ride>;
  };
}

Group.init(
  {
    id: {
      type: DataTypes.INTEGER.UNSIGNED,
      autoIncrement: true,
      primaryKey: true,
    },
    leaderUserId: {
      type: DataTypes.INTEGER.UNSIGNED,
      allowNull: false,
    },
    homeId: {
      type: DataTypes.INTEGER.UNSIGNED,
      allowNull: false,
    },
    destinationId: {
      type: DataTypes.INTEGER.UNSIGNED,
      allowNull: false,
    },
    name: {
      type: new DataTypes.STRING(128),
      allowNull: false,
    },
    currentOccupancy: {
      type: DataTypes.SMALLINT.UNSIGNED,
      allowNull: false,
    },
    hidden: {
      type: DataTypes.BOOLEAN,
      allowNull: false,
    },
    onRequest: {
      type: DataTypes.BOOLEAN,
      allowNull: false,
    },
    bearing: {
      type: DataTypes.DOUBLE,
      allowNull: false,
    },
  },
  {
    sequelize,
    tableName: 'groups',
  }
);
